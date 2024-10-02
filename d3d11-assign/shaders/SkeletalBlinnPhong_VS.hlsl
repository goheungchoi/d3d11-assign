// BlinnPhong_VS.hlsl
#include "MatrixInverse.hlsli"

#define MAX_BONES 100
#define MAX_BONE_WEIGHTS 8

cbuffer PerFrame : register(b0)
{
	matrix viewProjection;
}

cbuffer PerObject : register(b1)
{
	matrix model; // 16 x 4 = 64 bytes
	matrix inverseTransposeModel;	// 64 bytes
	//-----------------------------------
	matrix finalBoneTransforms[MAX_BONES];	// 64 x 100 = 6400 bytes
};

struct VS_INPUT
{
	float3 Position : POSITION;
	float2 TexCoord : TEXCOORD;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	//-----------------------------------
	int boneIds[MAX_BONE_WEIGHTS] : BONE_IDS;
	float boneWeights[MAX_BONE_WEIGHTS] : BONE_WEIGHTS;
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
	
	matrix boneTransform = matrix(
	 0.f, 0.f, 0.f, 0.f,
	 0.f, 0.f, 0.f, 0.f,
	 0.f, 0.f, 0.f, 0.f,
	 0.f, 0.f, 0.f, 0.f
	);
	
	[unroll]
	for (int i = 0; i < MAX_BONE_WEIGHTS; ++i)
	{
		if (input.boneIds[i] == -1)
			continue;
		boneTransform += input.boneWeights[i] * finalBoneTransforms[input.boneIds[i]];
	}
	float4 totalPos = mul(pos, boneTransform);
	
	//matrix normalTransform = inverse(boneTransform) * inverseTransposeModel;
	matrix normalTransform = boneTransform;
	float3 T = normalize(mul(float4(input.Tangent, 0.f), normalTransform)).xyz;
	float3 N = normalize(mul(float4(input.Normal, 0.f), normalTransform)).xyz;
	// re-orthogonalize T with respect to N
	T = normalize(T - dot(T, N) * N);
	float3 B = cross(N, T);
	float3x3 TBN = float3x3(T, B, N);
	
	VS_OUTPUT output;
	matrix mvp = mul(model, viewProjection);
	output.Position = mul(totalPos, mvp);
	output.WorldPosition = mul(totalPos, model);
	output.TexCoord = input.TexCoord;
	output.Normal = N;
	output.TBN = TBN;
	return output;
}
