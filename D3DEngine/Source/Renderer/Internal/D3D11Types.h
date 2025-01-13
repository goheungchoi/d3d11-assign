#pragma once

#include "D3DEngine/EngineCommon.h"

namespace DX {

inline ID3D11UnorderedAccessView* const nullUAV[] = {nullptr};
inline ID3D11Buffer* const nullBuffer[] = {nullptr};

struct FrameData {
  XMMATRIX view, invView;
  XMMATRIX proj, invProj;
  XMMATRIX viewProj;
};

struct ObjectData {
  XMMATRIX world;
};

enum class MaterialPass : uint8_t {
	kOpaque,
	kTransparent,
	kLight,
};

struct MaterialInstance {
  MaterialPass passType;

  class PipelineState* pipeline;

	std::vector<Handle> textureSet;
  std::vector<ComPtr<ID3D11Buffer>> cbSet;
};

struct MeshBuffer {
  ComPtr<ID3D11Buffer> vertexBuffer;
  ComPtr<ID3D11Buffer> indexBuffer;
  UINT stride;
  UINT offset;
  UINT numElements;

	DX::ObjectData _objectData;
  ComPtr<ID3D11Buffer> _objectDataCB;

	Handle materialInstance;
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

}  // namespace DX
