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
  ComPtr<ID3D11Texture2D> colorTexture;
  ComPtr<ID3D11Texture2D> depthStencilTexture;
  ComPtr<ID3D11RenderTargetView> rtv;
  ComPtr<ID3D11ShaderResourceView> colorSRV;
  ComPtr<ID3D11DepthStencilView> dsv;
  ComPtr<ID3D11ShaderResourceView> depthSRV;
  UINT width, height;
  UINT samples;
};

struct ShaderProgram {
  ComPtr<ID3D11VertexShader> vertexShader;
  ComPtr<ID3D11PixelShader> pixelShader;
  ComPtr<ID3D11InputLayout> inputLayout;
};

struct ComputeProgram {
  ComPtr<ID3D11ComputeShader> computeShader;
};

struct TextureBuffer {
  ComPtr<ID3D11Texture2D> texture;
  ComPtr<ID3D11ShaderResourceView> srv;
  ComPtr<ID3D11UnorderedAccessView> uav;
  UINT width, height;
  UINT levels;
};