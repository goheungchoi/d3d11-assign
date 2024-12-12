// Physically Based Rendering
// Copyright (c) 2017-2018 Micha©© Siejak

// Environment skybox.

// Vertex shader
cbuffer TransformConstants : register(b0)
{
	float4x4 view;
	float4x4 proj;
	float4x4 sceneRotationMatrix;
};

struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float3 texCoords : TEXCOORD;
};

PixelShaderInput main(float3 position : POSITION)
{
	PixelShaderInput vout;
	vout.texCoords = position.xyz;
	
	float3 pos = mul(position, (float3x3) view);
	vout.position = mul(float4(pos, 1.f), proj);

	return vout;
}