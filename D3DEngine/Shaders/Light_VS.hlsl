// Light_VS.hlsl

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
	float2 texcoord : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

VS_OUTPUT main(uint vertexID : SV_VertexID)
{
	VS_OUTPUT output;

    // Hardcoded vertex positions for a full-screen triangle
  float2 pos[3] = {
      float2(-1.0f, -1.0f),  // Bottom-left
      float2(-1.0f, 3.0f),   // Top-left extended to cover full screen
      float2(3.0f, -1.0f)    // Bottom-right extended to cover full screen
  };

  // UVs corresponding to the full screen
  float2 uv[3] = {float2(0.0f, 0.0f), float2(0.0f, 2.0f), float2(2.0f, 0.0f)};

  // Use vertexID to select the correct triangle corner
  output.position = float4(pos[vertexID], 0.0f, 1.0f);  // Clip space position
  output.texcoord = uv[vertexID];  // Texture UV coordinates
		
	return output;
}
