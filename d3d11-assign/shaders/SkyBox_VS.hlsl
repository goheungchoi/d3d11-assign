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
	
	float4 pos = float4(mul(position, (float3x3) view), 1.f);
	
	pos = mul(pos, float4x4(100.f, 0.f, 0.f, 0.f,
													0.f, 100.f, 0.f, 0.f,
													0.f, 0.f, 100.f, 0.f,
													0.f, 0.f, 0.f, 1.f));
	
	vout.position = mul(pos, proj);

	return vout;
}