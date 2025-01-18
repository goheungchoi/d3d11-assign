// Light_PS.hlsl

#define UNDEFINED  0;
#define DIRECTIONAL 1;
#define POINT 2;
#define SPOT 3;

static const float PI = 3.141592;
static const float Epsilon = 0.00001;

// Constant normal incidence Fresnel factor for all dielectrics.
static const float3 Fdielectric = 0.04;

Texture2D GBuffer_position : register(t0);
Texture2D GBuffer_color : register(t1);
Texture2D GBuffer_normal : register(t2);
Texture2D GBuffer_metalRoughness : register(t3);
Texture2D GBuffer_depth : register(t4);

TextureCube specularTexture : register(t5);
TextureCube irradianceTexture : register(t6);
Texture2D specularBRDF_LUT : register(t7);

TextureCube shadowMap : register(t8);

SamplerState defaultSampler : register(s0);
SamplerState wrapSampler : register(s1);
SamplerState clampSampler : register(s2);
SamplerState pointSampler : register(s3);

cbuffer FrameData : register(b0)
{
	matrix view;
	matrix invView;
	matrix proj;
	matrix invProj;
	matrix viewProj;
};

struct LightData
{
	float4 position; // [x,y,z,1] - position, [x,y,z,0] - direction
	float4 radiance; // r, g, b, intensity

	float spotAngle;
	float constantAttenuation;
	float linearAttenuation;
	float quadraticAttenuation;

	float nearPlane;
  float farPlane;
	bool enabled;
	uint type;
};

cbuffer LightShadingConstants : register(b2)
{
	LightData light;
	
	float4 eyePosition;
	
	float exposure;
	float gamma;
	bool useIBL;
	bool usePCF;

	float screenWidth;
  float screenHeight;
};

// Reconstruct view space position from depth
float3 ViewPositionFromDepth(float2 texcoord, float depth)
{
  // Get x/w and y/w from the viewport position
	float3 projectedPos = float3(texcoord, depth) * 2.0f - 1.0f;;

  // Transform by the inverse projection matrix
	float4 positionVS = mul(float4(projectedPos, 1.0f), invProj);

  // Divide by w to get the view-space position
	return positionVS.xyz / positionVS.w;
}

// GGX/Towbridge-Reitz normal distribution function.
// Uses Disney's reparametrization of alpha = roughness^2.
float ndfGGX(float cosLh, float roughness)
{
	float alpha = roughness * roughness;
	float alphaSq = alpha * alpha;

	float denom = (cosLh * cosLh) * (alphaSq - 1.0) + 1.0;
	return alphaSq / (PI * denom * denom);
}

// Single term for separable Schlick-GGX below.
float gaSchlickG1(float cosTheta, float k)
{
	return cosTheta / (cosTheta * (1.0 - k) + k);
}

// Schlick-GGX approximation of geometric attenuation function using Smith's method.
float gaSchlickGGX(float cosLi, float cosLo, float roughness)
{
	float r = roughness + 1.0;
	float k = (r * r) / 8.0; // Epic suggests using this roughness remapping for analytic lights.
	return gaSchlickG1(cosLi, k) * gaSchlickG1(cosLo, k);
}

// Shlick's approximation of the Fresnel factor. 
float3 fresnelSchlick(float3 F0, float cosTheta)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// Returns number of mipmap levels for specular IBL environment map.
uint querySpecularTextureLevels()
{
	uint width, height, levels;
	specularTexture.GetDimensions(0, width, height, levels);
	return levels;
}

float depthToWorldSpaceDistance(float depth, float nearPlane, float farPlane) {
  // In reversed Z, depth = 1.0 corresponds to near plane, and 0.0 to far plane
  return (nearPlane * farPlane) / ((farPlane - nearPlane) * depth + nearPlane);
}

float shadowMapVisibility(float3 P, float3 L, float3 N)
{
	/*float4 PLw = mul(float4(P, 1.0), light.lightSpace);
	float3 PL = (PLw.xyz / PLw.w) * 0.5 + 0.5;
	if (PL.z > 1.0 || PL.z < 0.0)
		return 0.0;
	float closestDepth = shadowMap.Sample(defaultSampler, PL).r;
	float bias = max(0.005 * (1.0 - dot(N, L)), 0.003);
	return ((PL.z - bias) > closestDepth) ? 1.0 : 0.0;*/

	float3 lightToFrag = P - L;
  float distanceToFrag = length(lightToFrag);
  float3 directionToFrag = normalize(lightToFrag);
   
	float sampledDepth = shadowMap.Sample(pointSampler, directionToFrag).r;
  float closestDistance =
      depthToWorldSpaceDistance(sampledDepth, light.nearPlane, light.farPlane);

  float bias = max(0.005 * (1.0 - dot(N, L)), 0.0003);
  return ((distanceToFrag - bias) > closestDistance) ? 1.0 : 0.0;
}

float3 Uncharted2ToneMapping(float3 color)
{
	float A = 0.22; //0.15;
	float B = 0.30; //0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.01; //0.02;
	float F = 0.30; //0.30;
	float W = 11.2;
	float exposure = 2.;
	color *= exposure;
	color = ((color * (A * color + C * B) + D * E) / (color * (A * color + B) + D * F)) - E / F;
	float white = ((W * (A * W + C * B) + D * E) / (W * (A * W + B) + D * F)) - E / F;
	color /= white;
  color = pow(color, float3(1.f / gamma, 1.f / gamma, 1.f / gamma));
	return color;
}

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

float4 main(PS_INPUT input) : SV_Target
{
	float4 worldPosition = GBuffer_position.Sample(pointSampler, input.texcoord);
	float3 albedo = GBuffer_color.Sample(pointSampler, input.texcoord).rgb;
	float alpha = GBuffer_color.Sample(pointSampler, input.texcoord).a;
	float metalness = GBuffer_metalRoughness.Sample(pointSampler, input.texcoord).r;
	float roughness = GBuffer_metalRoughness.Sample(pointSampler, input.texcoord).g;
	float depth = GBuffer_depth.Sample(pointSampler, input.texcoord).r;
	
	// Outgoing light dir
	float3 Lo = normalize(eyePosition.xyz - worldPosition.xyz);
	
	// Normal
	float3 N = GBuffer_normal.Sample(pointSampler, input.texcoord).xyz;
	
	float NdotLo = max(0.f, dot(N, Lo));
	
	// Specular reflection
	float3 R = 2.0 * NdotLo * N - Lo;
	
	// Fresnel reflectance 
	float3 F0 = lerp(Fdielectric, albedo, metalness);
	
	// Direct lighting
	float3 directLighting = 0.0;
  float3 Li = normalize(light.position.xyz - worldPosition.xyz);
	
	float3 Lradiance = light.radiance.rgb * light.radiance.a * light.enabled;
		
	// Half-vector between Li and Lo
	float3 Lh = normalize(Li + Lo);
		
	// Angles between surface normal and light vectors.
	float NdotLi = max(0.0, dot(N, Li));
	float NdotLh = max(0.0, dot(N, Lh));
		
	// Fresnel term for direct lighting
	float3 F = fresnelSchlick(F0, max(0.0, dot(Lh, Lo)));
	// Normal distribution for specular BRDF
	float D = ndfGGX(NdotLh, max(0.01, roughness));
	// Geometric attenuation for specular BRDF
	float G = gaSchlickGGX(NdotLi, NdotLo, roughness);
		
	// Diffuse scattering happens due to light being refracted multiple times by a dielectric medium.
	// Metals on the other hand either reflect or absorb energy, so diffuse contribution is always zero.
	// To be energy conserving we must scale diffuse BRDF contribution based on Fresnel factor & metalness.
	float3 kd = lerp(float3(1.0, 1.0, 1.0) - F, float3(0.f, 0.f, 0.f), metalness);
		
	// Lambert diffuse BRDF
	float3 diffuseBRDF = kd * albedo / PI;
		
	// Cook-Torrance specular microfacet BRDF
	float3 specularBRDF = (F * D * G) / max(Epsilon, 4.0 * NdotLi * NdotLo);
		
	// Total
	directLighting += (diffuseBRDF + specularBRDF) * Lradiance * NdotLi;
	
	
	// Ambient lighting
	float3 ambientLighting = 0.f;
	if (useIBL > 0)
	{
		// Sample diffuse irradiance at normal direction
		float3 irradiance = irradianceTexture.Sample(defaultSampler, N).rgb;
		
		// Calculate Fresnel term for ambient lighting.
		// Since we use pre-filtered cubemap(s) and irradiance is coming from many directions
		// use cosLo instead of angle with light's half-vector (cosLh above).
		// See: https://seblagarde.wordpress.com/2011/08/17/hello-world/
		float3 F = fresnelSchlick(F0, NdotLo);
		
		// Diffuse contribution factor
		float3 kd = lerp(1.0 - F, 0.0, metalness);
		
		// Irradiance map contains exitant radiance assuming Lambertian BRDF, no need to scale by 1/PI here either.
		float3 diffuseIBL = kd * albedo * irradiance;
		
		// Sample pre-filtered specular reflection environment at correct mipmap level
		uint specularTextureLevels = querySpecularTextureLevels();
		float3 specularIrradiance = specularTexture.SampleLevel(defaultSampler, R, roughness * specularTextureLevels).rgb;
		
		// Split-sum approximation factors fro Cook-Torrance specular BRDF.
		float2 specularBRDF = specularBRDF_LUT.Sample(clampSampler, float2(NdotLo, roughness)).rg;
		
		// Total specular IBL
		float3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;
		
		// Total ambient lighting
		ambientLighting = diffuseIBL + specularIBL;
	}
	
	// Shadow map visibility term
  float vis = shadowMapVisibility(worldPosition.xyz, light.position.xyz, N);
  float3 result = (1.0 - vis) * directLighting + ambientLighting * 0.1f;
	
	// result = Uncharted2ToneMapping(exposure*result);

	return float4(result, 1.0);
}
