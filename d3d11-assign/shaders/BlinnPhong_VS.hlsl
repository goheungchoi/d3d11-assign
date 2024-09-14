cbuffer PerFrame : register(b0)
{
	matrix viewProjection;
}

cbuffer PerObject : register(b1)
{
	matrix model;
};

struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD;
};

struct PS_OUTPUT
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD;
};

PS_OUTPUT main(VS_INPUT input)
{
	float4 pos = float4(input.Position, 1.f);
	matrix mvp = mul(viewProjection, model);
	pos = mul(pos, mvp);
	
	PS_OUTPUT output;
	output.Position = pos;
	output.Normal = input.Normal;
	output.TexCoord = input.TexCoord;
	return output;
}