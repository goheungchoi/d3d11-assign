#pragma once

#include "common.h"

#include "D3DEngine/Renderer/D3D11Renderer.h"

#define DECLSPEC_CBUFFER_ALIGN __declspec(align(16))

#define NUM_LIGHTS 3

DECLSPEC_CBUFFER_ALIGN
struct cbTransformConstants {
	XMMATRIX view;
	XMMATRIX proj;
	XMMATRIX sceneRotation;
  XMMATRIX shadowViewProj;
};

struct Light {
	Vector4 direction;			// 16 bytes
	Vector4 radiance;				// 16 bytes
	UINT enabled{ false };	// 4 byptes
	UINT padding[3];				// 12 bytes
};												// Total: 48 bytes

DECLSPEC_CBUFFER_ALIGN
struct cbShadingConstants {
	Light lights[NUM_LIGHTS];	// 144 bytes
	Vector4 eyePosition;
	UINT useIBL;
  FLOAT gamma;
  FLOAT g_metalness;
  FLOAT g_roughness;
};

inline float g_camDist{150.f};
inline XMVECTOR g_camPos{0.f, 0.f, 150.f, 0.f};
