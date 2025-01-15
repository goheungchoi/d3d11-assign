#pragma once

#include "D3DEngine/EngineCommon.h"

namespace DX {

inline ID3D11UnorderedAccessView* const nullUAV[] = {nullptr};
inline ID3D11Buffer* const nullBuffer[] = {nullptr};

struct cbFrameData {
  XMMATRIX view, invView;
  XMMATRIX proj, invProj;
  XMMATRIX viewProj;
};

struct cbObjectData {
  XMMATRIX world;
};

struct cbLightData {
  Vector4 components;  // [x,y,z,1] - position, [-x,-y,-z,0] - direction
  Vector4 radiance;    // r, g, b, intensity

  float spotAngle;
  float constantAttenuation;
  float linearAttenuation;
  float quadraticAttenuation;

  LightType type;
  bool enabled;
};

struct cbShadingMaterial {
  XMVECTOR albedoFactor;
  float metallicFactor;
  float roughnessFactor;
  float emissiveFactor;
  float alphaCutoff;
  ////////////////////////
};

struct TextureBuffer {
  DXGI_FORMAT format;
  ComPtr<ID3D11Texture2D> texture;
  ComPtr<ID3D11ShaderResourceView> srv;
  ComPtr<ID3D11UnorderedAccessView> uav;
  UINT width, height;
  UINT levels;
};

struct CubeTextureBuffer {
  DXGI_FORMAT format;
  ComPtr<ID3D11Texture2D> texture;
  ComPtr<ID3D11ShaderResourceView> srv;
  ComPtr<ID3D11UnorderedAccessView> uav;
  UINT width, height;
  UINT levels, faces;
};

struct DepthStencilBuffer {
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

enum class MaterialPass : uint8_t {
  kOpaque,
  kTransparent,
  kLight,
};

struct MaterialInstance {
  MaterialPass passType;

  class PipelineState* pipeline;

	// TODO: std::unordered_map</* slot number */ UINT, DX::TextureBuffer>
	// TODO: std::unordered_map</* slot number */ UINT, ConstantBuffer>
  std::vector<DX::TextureBuffer> textureSet;
  std::vector<ComPtr<ID3D11SamplerState>> samplerSet;
  std::vector<ComPtr<ID3D11Buffer>> cbSet;
};

struct MeshBuffer {
  ComPtr<ID3D11Buffer> vertexBuffer;
  ComPtr<ID3D11Buffer> indexBuffer;
  UINT stride;
  UINT offset;
  UINT numIndices;

  DXGI_FORMAT indexFormat;

  Handle materialInstance;
};

struct LightInstance {
  std::vector<DepthStencilBuffer> shadowMaps;
  class FrameBuffer* shadowMapFrameBuffer;
};

}  // namespace DX
