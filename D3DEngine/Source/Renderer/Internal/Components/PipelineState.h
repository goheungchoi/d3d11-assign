#pragma once

#include "Renderer/Internal/D3D11Common.h"

namespace DX {

using StateFlags = uint8_t;
constexpr bool kDisabled = false;
constexpr bool kEnabled = true;

enum IATopologyStateFlag : uint8_t {
  kPointList = 0,
  kTriangleList = 1
};  // 1 bit

enum IAInputLayoutFlagBit : uint8_t {
  kNormal = 1U << 0,
  kTangent = 1U << 1,
  kBitangent = 1U << 2,
  kTexcoord = 1U << 3,
  kColor = 1U << 4
};  // 5 bits
using IAInputLayoutFlags = StateFlags;

enum RSFillModeFlag : uint8_t { 
	kSolid = 0x0, 
	kWireFrame = 0x1, 
	kPoint = 0x2 
};

enum RSCullModeFlag : uint8_t { kNone = 0x0, kFront = 0x1, kBack = 0x2 };

enum RSFrontClockwiseFlag : uint8_t {
  kFrontClockwise = 0,
  kFrontCounterClockwise = 1
};

enum RSMultisampleCountFlag : uint8_t {
  kSampleCount_1_Bit = 0x1,
  kSampleCount_2_Bit = 0x2,
  kSampleCount_4_Bit = 0x3,
  kSampleCount_8_Bit = 0x4,
  kSampleCount_16_Bit = 0x5,
  kSampleCount_32_Bit = 0x6,
};

using RSDepthClipEnabled = bool;
using RSScissorEnabled = bool;

using OMDepthEnabled = bool;

enum OMDepthCompOp : uint8_t {
  kDepthComp_Never = 0x1,
  kDepthComp_Less = 0x2,
  kDepthComp_Equal = 0x3,
  kDepthComp_LessEqual = 0x4,
  kDepthComp_Greater = 0x5,
  kDepthComp_NotEqual = 0x6,
  kDepthComp_GreaterEqual = 0x7,
  kDepthComp_Always = 0x8
};

enum OMBlendMode : uint8_t {
  kNoBlend = 1U << 0,
  kAdditiveBlend = 1U << 1,
  kAlphaBlend = 1U << 2,
};

struct PipelineStateAbstract {
  IATopologyStateFlag topology : 1;
  IAInputLayoutFlags inputLayout : 5;

  RSFillModeFlag fill : 2;
  RSCullModeFlag cull : 2;
  RSFrontClockwiseFlag frontClockwise : 1;
  RSMultisampleCountFlag multisampleCount : 4;
  RSDepthClipEnabled depthClipEnabled : 1;
	RSScissorEnabled scissorEnabled : 1;

  OMDepthEnabled depthEnabled : 1;
  OMDepthCompOp depthCompOp : 4;

  OMBlendMode blendMode : 2;

	Handle vertexShader;
  Handle pixelShader;
};

class PipelineState {
  PipelineStateAbstract _stateAbstract;

	// 
	D3D11_PRIMITIVE_TOPOLOGY _topology;
  ComPtr<ID3D11InputLayout> _layout;

  D3D11_VIEWPORT _viewport;
  D3D11_RECT _scissor;

	ComPtr<ID3D11RasterizerState> _rasterizerState;

  ComPtr<ID3D11VertexShader> _vs;
  ComPtr<ID3D11PixelShader> _ps;

	ComPtr<ID3D11BlendState> _blendState;
  ComPtr<ID3D11DepthStencilState> _depthState;

  std::vector<DXGI_FORMAT> _colorAttachmentFormats;
  DXGI_FORMAT _depthStencilAttachmentFormat{DXGI_FORMAT_UNKNOWN};

	friend class PipelineStateBuilder;

 public:
  // TODO:
  PipelineStateAbstract GetAbstract() { return _stateAbstract; }

	D3D11_VIEWPORT GetViewport() { return _viewport; }

	ID3D11InputLayout* GetInputLayout() { return _layout.Get(); }

	ID3D11VertexShader* GetVertexShader() { return _vs.Get(); }
  ID3D11PixelShader* GetPixelShader() { return _ps.Get(); }
	
	UINT GetColorAttachmentCount() const { 
		return _colorAttachmentFormats.size();
	}

	const std::vector<DXGI_FORMAT> GetColorAttachmentFormats() const {
    return _colorAttachmentFormats;
	}
};

class PipelineStateBuilder {
  D3D_PRIMITIVE_TOPOLOGY topology;
  D3D11_RASTERIZER_DESC rasterizerDesc;
  D3D11_DEPTH_STENCIL_DESC depthStencilDesc;

 public:
  PipelineStateBuilder& IAInputTopology(D3D_PRIMITIVE_TOPOLOGY topology);
  PipelineStateBuilder& IAInputLayout(IAInputLayoutFlags flags);

	PipelineStateBuilder& VSSetVertexShader(Handle vs);

	PipelineStateBuilder& RSSetFillMode(D3D11_FILL_MODE fillMode);
  PipelineStateBuilder& RSSetCullMode(D3D11_CULL_MODE cullMode);
  PipelineStateBuilder& RSEnableFrontFaceCounterClockwise();
  PipelineStateBuilder& RSEnableDepthClipping();
  PipelineStateBuilder& RSEnableScissor(D3D11_RECT scissor);
  PipelineStateBuilder& RSDisableMultisample();
  PipelineStateBuilder& RSEnableMultisample(UINT samples);

	PipelineStateBuilder& PSSetPixelShader(Handle ps);

  PipelineStateBuilder& OMDisableDepthTesting();
  PipelineStateBuilder& OMEnableDepthTesting(D3D11_COMPARISON_FUNC compOp);

  PipelineStateBuilder& OMDisableBlending();
  PipelineStateBuilder& OMEnableAdditiveBlending();
  PipelineStateBuilder& OMEnableAlphaBlending();

  PipelineState Build();

  // TODO:
};

}  // namespace DX

namespace std {
template<>
struct hash<DX::PipelineStateAbstract> {
	std::size_t operator()(const DX::PipelineStateAbstract& abstract) const noexcept {

	}
};

template <>
struct hash<DX::PipelineState> {
  std::size_t operator()(const DX::PipelineState& abstract) const noexcept {
    return 0;
	}
};
}  // namespace std

