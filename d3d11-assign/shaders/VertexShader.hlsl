
struct PS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR0;
};

PS_OUTPUT main(float4 Position : POSITION, float4 Color : COLOR)
{
	PS_OUTPUT output;
	output.Position = Position;
	output.Color = Color;
	return output;
}
