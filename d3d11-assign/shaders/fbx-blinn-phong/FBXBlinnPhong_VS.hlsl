cbuffer PerFrame : register(b0)
{
	matrix viewProjection;
}

cbuffer PerObject : register(b1)
{
	matrix model;
	matrix inverseTransposeModel;
};

struct VS_INPUT
{
	float3 Position : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 WorldPosition : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float3x3 TBN : TBNMATRIX;
};

VS_OUTPUT main(VS_INPUT input)
{
	float4 pos = float4(input.Position, 1.f);
	matrix mvp = mul(model, viewProjection);
	
	float3 T = normalize(mul(float4(input.Tangent, 0.f), inverseTransposeModel)).xyz;
	float3 N = normalize(mul(float4(input.Normal, 0.f), inverseTransposeModel)).xyz;
	T = normalize(T - dot(T, N) * N); // re-orthogonalize T with respect to N
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);
	
	VS_OUTPUT output;
	output.Position = mul(pos, mvp);
	output.WorldPosition = mul(pos, model);
	output.TexCoord = input.TexCoord;
	output.Normal = N;
	output.TBN = TBN;
	return output;
}