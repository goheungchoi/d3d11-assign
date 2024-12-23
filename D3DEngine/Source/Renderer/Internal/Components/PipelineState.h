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

enum RSFrontClockwise : uint8_t {
	kFrontClockwise = 0,
	kFrontCounterClockwise = 1
};

enum RSMultisample : uint8_t { kDisabled = 0, kEnabled = 1 };

struct PipelineState {
  IATopologyStateFlag topology : 1;
  IAInputLayoutFlag inputLayout : 5;
	

	RSFillModeFlag fill : 1;
  RSCullModeFlag cull : 2;
  RSFrontClockwise frontClockwise : 1;
  RSMultisample multisample : 1;


};


class PipelineStateBuilder {

public:

	void SetInputTopology(D3D_PRIMITIVE_TOPOLOGY topology);

};



