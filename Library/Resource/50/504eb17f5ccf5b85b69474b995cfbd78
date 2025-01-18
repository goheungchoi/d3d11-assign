// PointShadowMap_VS.hlsl

cbuffer PointLightTransform : register(b0) {
  matrix view;
  matrix proj;
};

struct VS_INPUT
{
	float3 position : POSITION;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	
	// NDC position
	matrix mvp = mul(view, proj);
	output.position = mul(float4(input.position, 1.0), mvp);
	
	return output;
}
