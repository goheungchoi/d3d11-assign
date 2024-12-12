#define NUM_LIGHTS 3

TextureCube envTexture : register(t0);
SamplerState defaultSampler : register(s0);

cbuffer ShadingConstants : register(b0) {
  struct {
    float4 direction;
    float4 radiance;
    float4 paddings;
  } lights[NUM_LIGHTS];
  float4 eyePosition;
  bool useIBL;
  float gamma;
  uint padding[2];
};

struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float3 texCoords : TEXCOORD;
};

// Pixel shader
float4 main(PixelShaderInput pin) : SV_Target
{
  return pow(envTexture.SampleLevel(defaultSampler, pin.texCoords, 0), 1 / gamma);
}