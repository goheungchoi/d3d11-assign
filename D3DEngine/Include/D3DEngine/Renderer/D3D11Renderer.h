#pragma once

#include <d3d11_1.h>
#include <dxgi1_5.h>

#include "D3DEngine/EngineCommon.h"

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#include "D3D11Utils.h"
#include "D3DEngine/Core/Image.h"
#include "D3DEngine/Core/Mesh.h"

#define MAX_SAMPLE 16

inline ID3D11UnorderedAccessView* const nullUAV[] = {nullptr};
inline ID3D11Buffer* const nullBuffer[] = {nullptr};

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
  ComPtr<ID3D11ShaderResourceView> srv;
  ComPtr<ID3D11DepthStencilView> dsv;
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

struct Texture {
  ComPtr<ID3D11Texture2D> texture;
  ComPtr<ID3D11ShaderResourceView> srv;
  ComPtr<ID3D11UnorderedAccessView> uav;
  UINT width, height;
  UINT levels;
};

class D3D11Renderer {
  
  bool _vsyncEnabled{false};
  int _vram{0};
  char _gpuDesciption[128]{};

  const D3D_FEATURE_LEVEL _minFeatureLevel{D3D_FEATURE_LEVEL_10_0};
  D3D_FEATURE_LEVEL _featureLevel{};

  D3D11Renderer(const D3D11Renderer&) = delete;
  D3D11Renderer& operator=(const D3D11Renderer&) = delete;

 public:

  bool _useToneMappingAndGamma{false};

  D3D11Renderer() {}

  HRESULT Initialize(HWND hWnd, UINT width, UINT height);
  void Shutdown();

  void BeginDraw();
  void EndDraw();

  Color backgroundColor{0.f, 0.5f, 0.5f, 1.f};

  ID3D11Device* _device{nullptr};
  ID3D11Debug* _d3dDebug{nullptr};
  ID3D11DeviceContext* _context{nullptr};
  IDXGIFactory2* _dxgiFactory{nullptr};
  IDXGISwapChain1* _swapchain{nullptr};

  HRESULT CreateDeviceResources();
  HRESULT CreateDebugLayer();
  HRESULT CreateFactory();
  HRESULT CreateSwapchain(HWND hWnd, UINT width, UINT height);
  HRESULT CreateBackBufferView();
  HRESULT CreateFrameBuffers();
  HRESULT CreateToneMappingAndGammaCorrectionProgram();

  ID3D11Texture2D* _backbuffer{nullptr};
  D3D11_TEXTURE2D_DESC _backbufferDesc{};
  ID3D11RenderTargetView* _backbufferRTV{nullptr};

  ID3D11RasterizerState* _rasterizerState{nullptr};

  ID3D11Texture2D* _depthStencilBuffer{nullptr};
  ID3D11DepthStencilState* _depthStencilState{nullptr};
  ID3D11DepthStencilView* _depthStencilView{nullptr};

	FrameBuffer _framebuffer;
  FrameBuffer _resolveFramebuffer;
  ShaderProgram _toneMappingProgram;
  ComPtr<ID3D11SamplerState> _toneMappingSamplerState;

  D3D11_VIEWPORT _viewport{};

  HRESULT ConfigureDepthStencilBuffer();
  HRESULT ConfigureRasterizer();
  HRESULT ConfigureViewport();

  MeshBuffer CreateMeshBuffer(const std::shared_ptr<class Mesh>& mesh) const;
  ShaderProgram CreateShaderProgram(
      const std::vector<uint8_t>& vsBytecode,
      const std::vector<uint8_t>& psBytecode,
      const std::vector<D3D11_INPUT_ELEMENT_DESC>* inputLayoutDesc) const;
  ComputeProgram CreateComputeProgram(
      const std::vector<uint8_t>& csBytecode) const;
  ComPtr<ID3D11SamplerState> CreateSamplerState(
      D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode) const;

  Texture CreateTexture(UINT width, UINT height, DXGI_FORMAT format,
                        UINT levels = 0) const;
  Texture CreateTexture(const std::shared_ptr<class Image>& image,
                        DXGI_FORMAT format, UINT levels = 0) const;
  Texture CreateTextureCube(UINT width, UINT height, DXGI_FORMAT format,
                            UINT levels = 0) const;
  Texture CreateTextureCube(const std::string& path,
                            DXGI_FORMAT format, UINT levels = 0) const;

  void CreateTextureUAV(Texture& texture, UINT mipSlice) const;

  FrameBuffer CreateFrameBuffer(UINT width, UINT height, UINT samples,
                                DXGI_FORMAT colorFormat,
                                DXGI_FORMAT depthstencilFormat) const;
  void ResolveFrameBuffer(const FrameBuffer& srcfb, const FrameBuffer& dstfb,
                          DXGI_FORMAT format) const;

  ComPtr<ID3D11Buffer> CreateConstantBuffer(const void* data, UINT size) const;
  template <typename T>
  ComPtr<ID3D11Buffer> CreateConstantBuffer(const T* data = nullptr) const {
    static_assert(sizeof(T) == Utility::roundToPowerOfTwo(sizeof(T), 16));
    return CreateConstantBuffer(data, sizeof(T));
  }

	void CopyDataToDeviceBuffer(ComPtr<ID3D11Buffer>& buffer, const void* data);

  static ComPtr<ID3DBlob> CompileShader(const std::string& filename,
                                        const std::string& entryPoint,
                                        const std::string& profile);
};
