
#define MAX_LIGHTS 8

// Light types.

#define DIRECTIONAL_LIGHT 0

#define POINT_LIGHT 1

#define SPOT_LIGHT 2

Texture2D diffuseTexture : register(t0);
SamplerState diffuseSampler : register(s0);

Texture2D specularTexture : register(t1);
SamplerState specularSampler : register(s1);

Texture2D normalTexture : register(t2);
SamplerState normalSampler : register(s2);

struct Light
{
	float4 Position; // 16 bytes
  //----------------------------------- (16 byte boundary)
	float4 Direction; // 16 bytes
  //----------------------------------- (16 byte boundary)
	float4 Color; // 16 bytes
  //----------------------------------- (16 byte boundary)
	float SpotAngle; // 4 bytes
	float ConstantAttenuation; // 4 bytes
	float LinearAttenuation; // 4 bytes
	float QuadraticAttenuation; // 4 bytes
  //----------------------------------- (16 byte boundary)
	int LightType; // 4 bytes
	bool Enabled; // 4 bytes
	int2 Padding; // 8 bytes
  //----------------------------------- (16 byte boundary)
}; // Total:                           // 80 bytes (5 * 16)

cbuffer LightProperties : register(b1)
{
	float4 EyePosition; // 16 bytes
  //----------------------------------- (16 byte boundary)
	float4 GlobalAmbient; // 16 bytes
  //----------------------------------- (16 byte boundary)
	Light Lights[MAX_LIGHTS]; // 80 * 8 = 640 bytes
};  // Total:                           // 672 bytes (42 * 16)


struct PS_INPUT
{
	float4 Position : SV_POSITION;
	float3 Normal : NORMAL;
	float2 TexCoord : TEXCOORD;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	float4 color = diffuseTexture.Sample(diffuseSampler, input.TexCoord);
	return color;
}