// Geometry_VS.hlsl

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
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
	float2 texcoord : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 worldPosition : POSITION;
	float2 texcoord : TEXCOORD;
	float3x3 tangentBasis : TBASIS;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	output.worldPosition = mul(float4(input.position, 1.f), world);
	output.texcoord = input.texcoord;
	
	// Pass tangent space basis vectors (for normal mapping).
	float3x3 TBN = float3x3(input.tangent, input.bitangent, input.normal);
	output.tangentBasis = mul((float3x3) world, transpose(TBN));
	
	// NDC position
	matrix mvp = mul(world, viewProj);
	output.position = mul(float4(input.position, 1.0), mvp);
		
	return output;
}
