
cbuffer MatrixBuffer
{
	matrix model;
	matrix lightViewProjection;
};

struct VS_INPUT
{
	float3 Position : POSITION;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
};

VS_OUTPUT main(VS_INPUT input)
{
	float4 pos = float4(input.Position, 1.f);
	VS_OUTPUT output;
	output.Position = mul(mul(pos, model), lightViewProjection);
	return output;
}
