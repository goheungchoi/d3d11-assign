

cbuffer FrameData : register(b0)
{
	matrix view;
	matrix invView;
	matrix proj;
	matrix invProj;
	matrix viewProj;
};

cbuffer ObjectData : register(b1)
{
	matrix world;
};

struct VS_INPUT
{
	float3 position : POSITION;
	float2 texcoord : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	
	// NDC position
	matrix mvp = mul(world, viewProj);
	output.position = mul(float4(input.position, 1.0), mvp);
	output.texcoord = input.texcoord;
	
	return output;
}
