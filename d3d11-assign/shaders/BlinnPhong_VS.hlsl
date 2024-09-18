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
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
};

struct PS_OUTPUT
{
	float4 Position : SV_POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float3x3 TBN : TBNMATRIX;
};

PS_OUTPUT main(VS_INPUT input)
{
	float4 pos = float4(input.Position, 1.f);
	matrix mvp = mul(viewProjection, model);
	pos = mul(pos, mvp);
	
	float3 T = normalize(mul(float4(input.Tangent, 0.0), model)).xyz;
	float3 N = normalize(mul(float4(input.Normal, 0.0), model)).xyz;
	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);
	
	PS_OUTPUT output;
	output.Position = pos;
	output.TexCoord = input.TexCoord;
	output.Normal = input.Normal;
	output.TBN = TBN;
	return output;
}