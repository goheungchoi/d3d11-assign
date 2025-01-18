// PointShadowMap_PS.hlsl

cbuffer CameraData : register(b0) { 
	float nearPlane;
  float farPlane;
  uint2 padding;
};

struct PS_INPUT {
  float4 position : SV_POSITION;
};

float4 main(PS_INPUT input) : SV_Target {
  float linearDepth = input.position.z / input.position.w;  // Linear depth
  return float4(linearDepth, linearDepth, linearDepth, 1.0f);  // Debug output
}
