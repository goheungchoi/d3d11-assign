#pragma once

#include "Internal/D3D11Common.h"

#include "Internal/D3D11Types.h"

#define MAX_SAMPLE 16

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

	HRESULT 

  void Shutdown();

  void BeginDraw();
  void EndDraw();

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

	// Texture creation functions
  TextureBuffer CreateTexture(UINT width, UINT height, DXGI_FORMAT format,
                        UINT levels = 0) const;
  TextureBuffer CreateDDSTexture(const std::string& path,
													 DXGI_FORMAT format, UINT levels) const;
  TextureBuffer CreateTextureCube(UINT width, UINT height, DXGI_FORMAT format,
                            UINT levels = 0) const;
  TextureBuffer CreateTextureCube(const std::string& path,
                            DXGI_FORMAT format, UINT levels = 0) const;

  void CreateTextureUAV(TextureBuffer& texture, UINT mipSlice) const;

	// Frame buffer creation
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
};
