// Physically Based Rendering
// Copyright (c) 2017-2018 Micha©© Siejak

// Physically Based shading model: Lambetrtian diffuse BRDF + Cook-Torrance microfacet specular BRDF + IBL for ambient.

// This implementation is based on "Real Shading in Unreal Engine 4" SIGGRAPH 2013 course notes by Epic Games.
// See: http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf

cbuffer TransformConstants : register(b0)
{
	float4x4 view;
	float4x4 proj;
	float4x4 sceneRotation;	// Model Matrix
};

struct VS_INPUT
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
	float2 texcoord : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float3 worldPosition : POSITION;
	float2 texcoord : TEXCOORD;
	float3x3 tangentBasis : TBASIS;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
  output.worldPosition = mul(float4(input.position, 1.0) , sceneRotation).xyz;
	output.texcoord = input.texcoord;
	
	// Pass tangent space basis vectors (for normal mapping).
	float3x3 TBN = float3x3(input.tangent, input.bitangent, input.normal);
	output.tangentBasis = mul((float3x3) sceneRotation, transpose(TBN));
	
	// NDC position
	matrix viewProj = mul(view, proj);
	matrix mvp = mul(viewProj, sceneRotation);
	output.position = mul(float4(input.position, 1.0), mvp);
	
	return output;
}
