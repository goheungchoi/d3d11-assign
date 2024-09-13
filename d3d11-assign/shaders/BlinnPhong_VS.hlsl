cbuffer MatrixBuffer : register( b0 )
{
	matrix model;
	matrix view;
	matrix proj;
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
	PS_OUTPUT output;
	output.Position = input.Position;
	output.Normal = input.Normal;
	output.TexCoord = input.TexCoord;
	return output;
}