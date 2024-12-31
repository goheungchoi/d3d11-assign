#pragma once

inline ID3D11UnorderedAccessView* const nullUAV[] = {nullptr};
inline ID3D11Buffer* const nullBuffer[] = {nullptr};

struct MaterialInstance {


};

struct MeshBuffer {
  ComPtr<ID3D11Buffer> vertexBuffer;
  ComPtr<ID3D11Buffer> indexBuffer;
  UINT stride;
  UINT offset;
  UINT numElements;
};

struct FrameBuffer {
  DXGI_FORMAT colorFormat, depthFormat;
  ComPtr<ID3D11Texture2D> colorTexture;
  ComPtr<ID3D11Texture2D> depthStencilTexture;
  ComPtr<ID3D11RenderTargetView> rtv;
  ComPtr<ID3D11ShaderResourceView> colorSRV;
  ComPtr<ID3D11DepthStencilView> dsv;
  ComPtr<ID3D11ShaderResourceView> depthSRV;
  UINT width, height;
  UINT samples;
};

struct ComputeProgram {
  ComPtr<ID3D11ComputeShader> computeShader;
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

};
