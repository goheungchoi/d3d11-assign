TextureCube envTexture : register(t0);
SamplerState defaultSampler : register(s0);

struct PixelShaderInput
{
	float3 localPosition : POSITION;
	float4 pixelPosition : SV_POSITION;
};

// Pixel shader
float4 main(PixelShaderInput pin) : SV_Target
{
	float3 envVector = normalize(pin.localPosition);
	return envTexture.SampleLevel(defaultSampler, envVector, 0);
}