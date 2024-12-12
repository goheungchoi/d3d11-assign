// Physically Based Rendering
// Copyright (c) 2017-2018 Micha�� Siejak

// Physically Based shading model: Lambetrtian diffuse BRDF + Cook-Torrance microfacet specular BRDF + IBL for ambient.

// This implementation is based on "Real Shading in Unreal Engine 4" SIGGRAPH 2013 course notes by Epic Games.
// See: http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf

#include "GammaCorrection.hlsli"

#define NUM_LIGHTS 3

static const float PI = 3.141592;
static const float Epsilon = 0.00001;

// Constant normal incidence Fresnel factor for all dielectrics.
static const float3 Fdielectric = 0.04;

Texture2D albedoTexture : register(t0);
Texture2D normalTexture : register(t1);
Texture2D metalnessTexture : register(t2);
Texture2D roughnessTexture : register(t3);
TextureCube specularTexture : register(t4);
TextureCube irradianceTexture : register(t5);
Texture2D specularBRDF_LUT : register(t6);

SamplerState defaultSampler : register(s0);
SamplerState spBRDF_Sampler : register(s1);


cbuffer ShadingConstants : register(b0)
{
	struct
	{
		float4 direction;
		float4 radiance;
		bool enabled;
		uint3 paddings;
	} lights[NUM_LIGHTS];
	float4 eyePosition;
	bool useIBL;
	float gamma;
	float g_metalness;
	float g_roughness;
};

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

// Shlick's approximation of the Fresnel factor. �ּҰ� F0 , �ִ밪�� 1.0,1.0,1.0
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

struct PS_INPUT
{
	float4 position : SV_POSITION;
	float3 worldPosition : POSITION;
	float2 texcoord : TEXCOORD;
  float3x3 tangentBasis : TBASIS;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	float3 albedo = pow(albedoTexture.Sample(defaultSampler, input.texcoord).rgb, gamma);
	// float metalness = metalnessTexture.Sample(defaultSampler, input.texcoord).r;
  // float roughness = roughnessTexture.Sample(defaultSampler, input.texcoord).r;
	
	float metalness = g_metalness;
	float roughness = g_roughness;
	
	// Outgoing light dir
	float3 Lo = normalize(eyePosition.xyz - input.worldPosition);
	
	// Normal
	float3 N = normalTexture.Sample(defaultSampler, input.texcoord).rgb;
	N = normalize(2.0 * N - 1.0);
	N = normalize(mul(input.tangentBasis, N));
	
	float NdotLo = max(0.f, dot(N, Lo));
	
	// Specular reflection
	float3 R = 2.0 * NdotLo * N - Lo;
	
	// Fresnel reflectance 
	float3 F0 = lerp(Fdielectric, albedo, metalness);
	
	// Direct lighting
	float3 directLighting = 0.0;
	[unroll]
	for (int i = 0; i < NUM_LIGHTS; ++i) {
		float3 Li = -lights[i].direction;
		float3 Lradiance = lights[i].radiance * lights[i].enabled;
		
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
	}
	
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
		float2 specularBRDF = specularBRDF_LUT.Sample(spBRDF_Sampler, float2(NdotLo, roughness)).rg;
		
		// Total specular IBL
		float3 specularIBL = (F0 * specularBRDF.x + specularBRDF.y) * specularIrradiance;
		
		// Total ambient lighting
		ambientLighting = diffuseIBL + specularIBL;
	}
	
	float3 result = directLighting + ambientLighting;
	return pow(float4(result, 1.0), 1/gamma);
}