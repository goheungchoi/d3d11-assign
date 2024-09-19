
struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float4 Color : COLOR0;
};

VS_OUTPUT main(float4 Position : POSITION, float4 Color : COLOR)
{
	VS_OUTPUT output;
	output.Position = Position;
	output.Color = Color;
	return output;
}
