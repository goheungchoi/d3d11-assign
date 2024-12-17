struct PS_INPUT
{
	float4 position : SV_POSITION;
};

float4 main(PS_INPUT input) : SV_TARGET
{
	// clip(-1.f);
	float depth = input.position.z / input.position.w;

	return float4(depth, depth, depth, 1.f);
}