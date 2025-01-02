#pragma once

#include "Renderer/Internal/D3D11Common.h"

using StateFlags = uint64_t;

enum IATopologyStateFlag : uint8_t { 
	kPointList = 0,
	kTriangleList = 1 
};	// 1 bit

enum IAInputLayoutFlag : uint8_t {
  kNormal = 1U << 0,
  kTangent = 1U << 1,
  kBitangent = 1U << 2,
  kTexcoord = 1U << 3,
  kColor = 1U << 4
};	// 5 bits

enum RSFillModeFlag : uint8_t { 
	kSolid = 0,
	kWireFrame = 1
};

enum RSCullModeFlag : uint8_t { 
	kNone = 0x0, 
	kFront = 0x1, 
	kBack = 0x2
};

enum RSFrontClockwiseFlag : uint8_t {
	kFrontClockwise = 0,
	kFrontCounterClockwise = 1
};

enum RSMultisampleFlag : uint8_t {
  kSampleCount_1_Bit = 0x1,
  kSampleCount_2_Bit = 0x2,
  kSampleCount_4_Bit = 0x3,
  kSampleCount_8_Bit = 0x4,
  kSampleCount_16_Bit = 0x5,
  kSampleCount_32_Bit = 0x6,
};

enum OMDepthEnabled : uint8_t { kDisabled = 0, kEnabled = 1 };

enum OMBlendMode : uint8_t {
  kNoBlend = 1U << 0,
  kAdditiveBlend = 1U << 1,
  kAlphaBlend = 1U << 2,
};

struct PipelineStateFlags {
  IATopologyStateFlag topology : 1;
  IAInputLayoutFlag inputLayout : 5;
	
	RSFillModeFlag fill : 1;
  RSCullModeFlag cull : 2;
  RSFrontClockwiseFlag frontClockwise : 1;
  RSMultisampleFlag multisample : 4;

	OMDepthEnabled depthEnabled : 1;
  OMBlendMode blendMode : 2;
};


class PipelineState {
  PipelineStateFlags _stateFlags;

  ComPtr<ID3D11InputLayout> _layout;
  
	D3D11_VIEWPORT _viewport;
	
  Handle _vs;
	Handle _ps;

	DXGI_FORMAT colorAttachmentFormat;
  DXGI_FORMAT depthAttachmentFormat;
 public:
	 // TODO:
};

class PipelineStateBuilder {
  D3D11_RASTERIZER_DESC rasterizerDesc;
  D3D11_DEPTH_STENCIL_DESC depthStencilDesc;

 public:

	void SetInputTopology(D3D_PRIMITIVE_TOPOLOGY topology);

	void RSDisableMultisample();
  void RSEnableMultisample();

	void OMDisableDepthTesting();
  void OMEnableDepthTesting(D3D11_COMPARISON_FUNC compOp);

	void OMDisableBlending();
  void OMEnableAdditiveBlending();
  void OMEnableAlphaBlending();

	PipelineState Build();

	// TODO:
};
