#pragma once

#include "D3DEngine/EngineCommon.h"

namespace DX {

inline ID3D11UnorderedAccessView* const nullUAV[] = {nullptr};
inline ID3D11Buffer* const nullBuffer[] = {nullptr};

enum class MaterialPass : uint8_t {
	kOpaque,
	kTransparent,
	kLight,
};

struct MaterialInstance {
  MaterialPass passType;

  class PipelineState* pipeline;
};

struct MeshBuffer {
  ComPtr<ID3D11Buffer> vertexBuffer;
  ComPtr<ID3D11Buffer> indexBuffer;
  UINT stride;
  UINT offset;
  UINT numElements;
};

struct TextureBuffer {
  DXGI_FORMAT format;
  ComPtr<ID3D11Texture2D> texture;
  ComPtr<ID3D11ShaderResourceView> srv;
  ComPtr<ID3D11UnorderedAccessView> uav;
  UINT width, height;
  UINT levels;
};

struct CubeTextureBuffer {};

struct DepthStensilBuffer {
  DXGI_FORMAT format;
  UINT width, height, samples;
  ComPtr<ID3D11Texture2D> texture;
  ComPtr<ID3D11DepthStencilView> dsv;
  ComPtr<ID3D11ShaderResourceView> srv;
};

struct RenderTargetBuffer {
  DXGI_FORMAT format;
  UINT width, height, samples;
  ComPtr<ID3D11Texture2D> texture;
  ComPtr<ID3D11RenderTargetView> rtv;
  ComPtr<ID3D11ShaderResourceView> srv;
};

struct FrameData {
  XMMATRIX view, invView;
  XMMATRIX proj, invProj;
  XMMATRIX viewProj;
};

struct ObjectData {
  XMMATRIX world;
};

}  // namespace DX
