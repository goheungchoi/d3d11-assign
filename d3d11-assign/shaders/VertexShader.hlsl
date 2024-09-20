
cbuffer PerFrame : register(b0)
{
	matrix viewProjection;
}

cbuffer PerObject : register(b1)
{
	matrix model;
	matrix inverseTransposeModel;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
};

VS_OUTPUT main(float3 Position : POSITION, float3 Color : COLOR)
{
	VS_OUTPUT output;
	output.Position = Position;
	output.Color = Color;
	return output;
}
