
cbuffer PerFrame : register(b0)
{
	matrix viewProjection;
}

cbuffer PerObject : register(b1)
{
	matrix model;
	matrix inverseTransposeModel;
	matrix modelViewProj;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR;
};

VS_OUTPUT main(float3 Position : POSITION, float3 Color : COLOR)
{
	matrix mvp = mul(model, viewProjection);
	
	float4 pos = mul(float4(Position, 1.f), mvp);
	
	VS_OUTPUT output;
	output.Position = pos;
	output.Color = float4(Color, 1.f);
	return output;
}
