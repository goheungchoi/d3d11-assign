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
	float3 localPosition : POSITION;
	float4 pixelPosition : SV_POSITION;
};

// Pixel shader
float4 main(PixelShaderInput pin) : SV_Target
{
	float3 envVector = normalize(pin.localPosition);
  return pow(envTexture.SampleLevel(defaultSampler, envVector, 0), 1 / gamma);
}