// Physically Based Rendering
// Copyright (c) 2017-2018 Micha©© Siejak

// Environment skybox.

// Vertex shader
cbuffer TransformConstants : register(b0)
{
	float4x4 viewProjectionMatrix;
	float4x4 skyProjectionMatrix;
	float4x4 sceneRotationMatrix;
};

struct PixelShaderInput
{
	float3 localPosition : POSITION;
	float4 pixelPosition : SV_POSITION;
};

PixelShaderInput main(float3 position : POSITION)
{
	PixelShaderInput vout;
	vout.localPosition = position;
  vout.pixelPosition = mul(float4(position, 1.0), skyProjectionMatrix);
	return vout;
}