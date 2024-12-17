cbuffer TransformConstants : register(b0)
{
	float4x4 view;
	float4x4 proj;
	float4x4 sceneRotation; // Model Matrix
	float4x4 shadowViewProj;
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
	matrix mvp = mul(sceneRotation, shadowViewProj);
	output.position = mul(float4(input.position, 1.0), mvp);
	
	return output;
}
