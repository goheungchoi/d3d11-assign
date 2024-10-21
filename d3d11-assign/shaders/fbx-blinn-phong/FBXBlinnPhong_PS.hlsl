
#define MAX_LIGHTS 8

// Light types.

#define UNDEFINED_LIGHT 0
#define DIRECTIONAL_LIGHT 1
#define POINT_LIGHT 2
#define SPOT_LIGHT 3

Texture2D diffuseTexture : register(t0);
SamplerState diffuseSampler : register(s0);

Texture2D specularTexture : register(t1);
SamplerState specularSampler : register(s1);

Texture2D normalTexture : register(t2);
SamplerState normalSampler : register(s2);

Texture2D emissiveTexture : register(t3);
SamplerState emissiveSampler : register(s3);

Texture2D opacityTexture : register(t4);
SamplerState opacitySampler : register(s4);

struct _Material
{
	float4 EmissiveColor; // 16 bytes
    //----------------------------------- (16 byte boundary)
	float4 AmbientColor; // 16 bytes
    //------------------------------------(16 byte boundary)
	float4 DiffuseColor; // 16 bytes
    //----------------------------------- (16 byte boundary)
	float4 SpecularColor; // 16 bytes
    //----------------------------------- (16 byte boundary)
	float Shineness; // 4 bytes
	bool UseDiffuseTexture; // 4 bytes
	float2 Padding; // 8 bytes
    //----------------------------------- (16 byte boundary)
	bool useSpecularTexture;
	bool useNormalTexture;
	bool useEmissiveTexture;
	bool useOpacityTexture;
}; // Total:               // 80 bytes ( 5 * 16 )

cbuffer MaterialProperties : register(b0)
{
	_Material Material;
};

struct Light
{
	float4 Position; // 16 bytes
  //----------------------------------- (16 byte boundary)
	float4 Direction; // 16 bytes
  //----------------------------------- (16 byte boundary)
	float4 Color; // 16 bytes
  //----------------------------------- (16 byte boundary)
	float SpotAngle; // 4 bytes
	float ConstantAttenuation; // 4 bytes
	float LinearAttenuation; // 4 bytes
	float QuadraticAttenuation; // 4 bytes
  //----------------------------------- (16 byte boundary)
	int LightType; // 4 bytes
	bool Enabled; // 4 bytes
	int2 Padding; // 8 bytes
  //----------------------------------- (16 byte boundary)
}; // Total:                           // 80 bytes (5 * 16)

cbuffer LightProperties : register(b1)
{
	float4 EyePosition; // 16 bytes
  //----------------------------------- (16 byte boundary)
	float4 GlobalAmbient; // 16 bytes
  //----------------------------------- (16 byte boundary)
	Light Lights[MAX_LIGHTS]; // 80 * 8 = 640 bytes
};  // Total:                           // 672 bytes (42 * 16)


float4 CalculateDiffuse(Light light, float3 L, float3 N)
{
	float NdotL = max(0, dot(N, L));
	
	return light.Color * NdotL;
}

float4 CalculateSpecular(Light light, float3 V, float3 L, float3 N)
{
	float3 H = normalize(V + L);
	float NdotH = max(0, dot(N, H));
	return light.Color * pow(NdotH, Material.Shineness);
}

float CalculateAttenuation(Light light, float d)
{
	return 1.0f /
		(light.ConstantAttenuation +
		light.LinearAttenuation * d +
		light.QuadraticAttenuation * d * d);
}

struct LightingResult
{
	float4 diffuse;
	float4 specular;
};

LightingResult CalculatePointLight(Light light, float3 V, float4 P, float3 N)
{
	LightingResult res;
	float3 L = (light.Position - P).xyz;
	float distance = length(L);
	
	L = L / distance; // Normalize
	
	float attenuation = CalculateAttenuation(light, distance);
	
	res.diffuse = attenuation * CalculateDiffuse(light, L, N);
	res.specular = attenuation * CalculateSpecular(light, V, L, N);
	
	return res;
}

LightingResult CalculateDirectionalLight(Light light, float3 V, float4 P, float3 N)
{
	LightingResult res;
	
	float3 L = -light.Direction.xyz;

	res.diffuse = CalculateDiffuse(light, L, N);
	res.specular = CalculateSpecular(light, V, L, N);
	
	return res;
}

float CalculateSpotCone(Light light, float3 L)
{
	float minCos = cos(light.SpotAngle);
	float maxCos = (minCos + 1.0f) / 2.0f;
	float cosAngle = dot(light.Direction.xyz, -L);
	return smoothstep(minCos, maxCos, cosAngle);
}

LightingResult CalculateSpotLight(Light light, float3 V, float4 P, float3 N)
{
	LightingResult res;
	
	float3 L = (light.Position - P).xyz;
	float distance = length(L);
	L = L / distance;
	
	float attenuation = CalculateAttenuation(light, distance);
	float spotIntensity = CalculateSpotCone(light, L);
	
	res.diffuse = spotIntensity * attenuation * CalculateDiffuse(light, L, N);
	res.specular = spotIntensity * attenuation * CalculateSpecular(light, V, L, N);
	
	return res;
}

LightingResult ComputeLighting(float4 P, float3 N)
{
	float3 V = normalize((EyePosition - P).xyz);
	
	LightingResult total;
	total.diffuse = float4(0.f, 0.f, 0.f, 0.f);
	total.specular = float4(0.f, 0.f, 0.f, 0.f);
	
	[unroll]
	for (int i = 0; i < MAX_LIGHTS; ++i)
	{
		LightingResult res;
		res.diffuse = float4(0.f, 0.f, 0.f, 0.f);
		res.specular = float4(0.f, 0.f, 0.f, 0.f);
		
		if (!Lights[i].Enabled)
			continue;
		
		switch (Lights[i].LightType)
		{
			case UNDEFINED_LIGHT:
				break;
			case DIRECTIONAL_LIGHT:
      {
					res = CalculateDirectionalLight(Lights[i], V, P, N);
				}
				break;
			case POINT_LIGHT:
			{
					res = CalculatePointLight(Lights[i], V, P, N);
				}
				break;
			case SPOT_LIGHT:
      {
					res = CalculateSpotLight(Lights[i], V, P, N);
				}
				break;
		}
		total.diffuse += res.diffuse;
		total.specular += res.specular;
	}
	
	total.diffuse = saturate(total.diffuse);
	total.specular = saturate(total.specular);
	
	return total;
}

struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float3x3 TBN : TBNMATRIX;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	float3 N = input.Normal.xyz;
	if (Material.useNormalTexture)
	{
		float4 normal = normalTexture.Sample(normalSampler, input.TexCoord);
		N = normal.xyz;
		N = N * 2.0 - 1.0;
		N = normalize(mul(N, input.TBN));
	}
		
	LightingResult lit = ComputeLighting(input.WorldPosition, N);
		
	float4 color = Material.DiffuseColor;
	if (Material.UseDiffuseTexture)
	{
		color = diffuseTexture.Sample(diffuseSampler, input.TexCoord);
	}
				
	// float4 emissive = Material.Emissive;	// TODO: Need emissive texture
	float4 Ia = color * GlobalAmbient;
	float4 Id = color * lit.diffuse;
		
	float4 Is = lit.specular * 0.4;
	if (Material.useSpecularTexture)
	{
		Is = lit.specular * specularTexture.Sample(specularSampler, input.TexCoord);
	}
	
	if (Material.useEmissiveTexture)
	{
		Ia += emissiveTexture.Sample(emissiveSampler, input.TexCoord);
	}
		
	float4 finalColor = pow(Ia + Id + Is, 1.0 / 2.2);
	
	if (Material.useOpacityTexture)
	{
		float opacity = opacityTexture.Sample(opacitySampler, input.TexCoord).a;
		finalColor.a *= opacity;
			
		if (finalColor.a <= 0.9)
			clip(-1);
	}
		
	return finalColor;
}