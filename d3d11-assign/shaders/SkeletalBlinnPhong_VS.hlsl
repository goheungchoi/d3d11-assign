// BlinnPhong_VS.hlsl
#define MAX_BONES 100
#define MAX_BONE_WEIGHTS 8

cbuffer PerFrame : register(b0)
{
	matrix viewProjection;
}

cbuffer PerObject : register(b1)
{
	matrix model; // 16 x 4 = 64 bytes
	matrix inverseTransposeModel; // 64 bytes
	//-----------------------------------
	matrix boneTransforms[MAX_BONES]; // 64 x 100 = 6400 bytes
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
	
	float4 totalPos = float4(0.f, 0.f, 0.f, 0.f);
	
	[unroll]
	for (int i = 0; i < MAX_BONE_WEIGHTS; ++i)
	{
		int boneId = input.boneIds[i];
		if (boneId < 0)
			continue;
		
		if (boneId >= MAX_BONES)
		{
			totalPos = pos;
			break;
		}
		
		float4 localPos = mul(pos, boneTransforms[boneId]);
		totalPos += localPos * input.boneWeights[i];
		
		float3 localNormal = mul(float4(input.Normal, 0.f), boneTransforms[boneId]);

	}
	
	float3 T = normalize(mul(float4(input.Tangent, 0.f), inverseTransposeModel)).xyz;
	float3 N = normalize(mul(float4(input.Normal, 0.f), inverseTransposeModel)).xyz;
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