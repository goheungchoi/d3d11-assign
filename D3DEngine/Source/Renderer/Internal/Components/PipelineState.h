#pragma once

#include "Renderer/Internal/D3D11Common.h"

#include <functional>

namespace DX {
struct PipelineStateAbstract;
}

bool operator==(const DX::PipelineStateAbstract& lhs,
                const DX::PipelineStateAbstract& rhs);

bool operator!=(const DX::PipelineStateAbstract& lhs,
                const DX::PipelineStateAbstract& rhs);

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
	kWireFrame = 0x1
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
  D3D11_VIEWPORT viewport;
  D3D11_RECT scissor;

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

  ComPtr<ID3D11DepthStencilState> _depthState;
	ComPtr<ID3D11BlendState> _blendState;

  std::vector<DXGI_FORMAT> _colorAttachmentFormats;
  std::optional<DXGI_FORMAT> _depthStencilAttachmentFormat;

	friend class PipelineStateBuilder;

 public:

  PipelineStateAbstract GetAbstract() const { return _stateAbstract; }

	const D3D11_VIEWPORT* GetViewport() const { return &_viewport; }
  const D3D11_RECT* GetScissor() const { return &_scissor; }

	D3D11_PRIMITIVE_TOPOLOGY GetInputTopology() const { return _topology; }
	ID3D11InputLayout* GetInputLayout() const { return _layout.Get(); }

	ID3D11VertexShader* GetVertexShader() const { return _vs.Get(); }
  ID3D11PixelShader* GetPixelShader() const { return _ps.Get(); }

	ID3D11RasterizerState* GetRasterizerState() const {
    return _rasterizerState.Get();
  }
	ID3D11DepthStencilState* GetDepthStencilState() const {
    return _depthState.Get();
  }
  ID3D11BlendState* GetBlendState() const { 
		return _blendState.Get();
	}

	
	UINT GetColorAttachmentCount() const { 
		return _colorAttachmentFormats.size();
	}

	const std::vector<DXGI_FORMAT>& GetColorAttachmentFormats() const {
    return _colorAttachmentFormats;
	}

	std::optional<DXGI_FORMAT> GetDepthStencilAttachmentFormat() const {
    return _depthStencilAttachmentFormat;
	}

	bool operator==(const PipelineState& other) const { 
		return _stateAbstract == other._stateAbstract &&
           _colorAttachmentFormats == other._colorAttachmentFormats &&
           _depthStencilAttachmentFormat == other._depthStencilAttachmentFormat;
	}

	bool operator!=(const PipelineState& other) const {
    return !(*this == other);
	}
};

class PipelineStateBuilder {
  PipelineStateAbstract _stateAbstract{};

  D3D_PRIMITIVE_TOPOLOGY _topology{D3D_PRIMITIVE_TOPOLOGY_UNDEFINED};
  std::vector<D3D11_INPUT_ELEMENT_DESC> _inputLayoutDesc;

	D3D11_VIEWPORT _viewport{};
  D3D11_RECT _scissor{};

  D3D11_RASTERIZER_DESC _rasterizerDesc{};
  D3D11_DEPTH_STENCIL_DESC _depthStencilDesc{};
  D3D11_BLEND_DESC _blendDesc{};

	std::vector<DXGI_FORMAT> _colorAttachmentFormats;
  std::optional<DXGI_FORMAT> _depthStencilAttachmentFormat;

	Handle _vs{};
  Handle _ps{};
 public:
  // TODO:

  PipelineStateBuilder& IAInputTopology(D3D_PRIMITIVE_TOPOLOGY topology);
  PipelineStateBuilder& IAInputLayout(IAInputLayoutFlags flags);

	PipelineStateBuilder& VSSetVertexShader(Handle vs);

	PipelineStateBuilder& RSSetViewport(UINT x, UINT y, UINT width, UINT height);
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

	PipelineStateBuilder& OMSetColorAttachmentFormats(std::initializer_list<DXGI_FORMAT> formats);
  PipelineStateBuilder& OMSetDepthAttachmentFormat(DXGI_FORMAT format);

	const PipelineStateAbstract& GetPipelineStateAbstract() const { return _stateAbstract; }

  PipelineState* Build(class RenderDevice* device);

  void Reset();
};

}  // namespace DX

namespace std {
template<>
struct hash<DX::PipelineStateAbstract> {
	std::size_t operator()(const DX::PipelineStateAbstract& state) const noexcept {
    std::size_t hash{0};

		// Combine hashes of basic fields
    hash_combine(hash, std::hash<int>{}(state.viewport.TopLeftX));
    hash_combine(hash, std::hash<int>{}(state.viewport.TopLeftY));
    hash_combine(hash, std::hash<int>{}(state.viewport.Width));
    hash_combine(hash, std::hash<int>{}(state.viewport.Height));
    hash_combine(hash, std::hash<float>{}(state.viewport.MinDepth));
    hash_combine(hash, std::hash<float>{}(state.viewport.MaxDepth));

    hash_combine(hash, std::hash<int>{}(state.scissor.left));
    hash_combine(hash, std::hash<int>{}(state.scissor.top));
    hash_combine(hash, std::hash<int>{}(state.scissor.right));
    hash_combine(hash, std::hash<int>{}(state.scissor.bottom));

    // Combine hashes of bitfields
    hash_combine(hash, std::hash<uint8_t>{}(state.topology));
    hash_combine(hash, std::hash<uint8_t>{}(state.inputLayout));
    hash_combine(hash, std::hash<uint8_t>{}(state.fill));
    hash_combine(hash, std::hash<uint8_t>{}(state.cull));
    hash_combine(hash, std::hash<uint8_t>{}(state.frontClockwise));
    hash_combine(hash, std::hash<uint8_t>{}(state.multisampleCount));
    hash_combine(hash, std::hash<bool>{}(state.depthClipEnabled));
    hash_combine(hash, std::hash<bool>{}(state.scissorEnabled));

    hash_combine(hash, std::hash<bool>{}(state.depthEnabled));
    hash_combine(hash, std::hash<uint8_t>{}(state.depthCompOp));
    hash_combine(hash, std::hash<uint8_t>{}(state.blendMode));

    // Combine hashes of Handle fields
    hash_combine(hash, std::hash<Handle>{}(state.vertexShader));
    hash_combine(hash, std::hash<Handle>{}(state.pixelShader));

    return hash;
	}

private:
  // Helper function to combine hashes
  static void hash_combine(std::size_t& seed, std::size_t value) noexcept {
    seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }
};

template <>
struct hash<DX::PipelineState> {
  std::size_t operator()(const DX::PipelineState& pipelineState) const noexcept {
    return std::hash<DX::PipelineStateAbstract>()(pipelineState.GetAbstract());
	}
};
}  // namespace std

