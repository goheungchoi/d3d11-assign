#include "D3DEngine/Renderer/D3D11Renderer.h"

HRESULT D3D11Renderer::Initialize(HWND hWnd, UINT width, UINT height) {
  HRESULT res = S_OK;

  res = CreateDeviceResources();
  if (FAILED(res)) return res;

  res = CreateDebugLayer();
  if (FAILED(res)) return res;

  res = CreateFactory();
  if (FAILED(res)) return res;

  res = CreateSwapchain(hWnd, width, height);
  if (FAILED(res)) return res;

  res = CreateBackBufferView();
  if (FAILED(res)) return res;

  res = CreateFrameBuffers();
  if (FAILED(res)) return res;

	res = CreateToneMappingAndGammaCorrectionProgram();
  if (FAILED(res)) return res;

  res = ConfigureDepthStencilBuffer();
  if (FAILED(res)) return res;

  res = ConfigureRasterizer();
  if (FAILED(res)) return res;

  res = ConfigureViewport();
  if (FAILED(res)) return res;

  return res;
}

void D3D11Renderer::Shutdown() {
#if _DEBUG
  ID3D11Debug* d3dDebug = nullptr;
  if (SUCCEEDED(
          _device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug))) {
    HRESULT res = d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
    if (FAILED(res)) {
      ErrorExit(GetComErrorString(res));
		}
  }
#endif  //  _DEBUG

  SafeRelease(&_device);
}

void D3D11Renderer::BeginDraw() {
  /*if (_useToneMappingAndGamma) {
    const float clear_color_with_alpha[] = {0.0f, 0.0f, 0.0f, 0.0f};
    _context->OMSetRenderTargets(1, _framebuffer.rtv.GetAddressOf(),
                                  _framebuffer.dsv.Get());

    _context->ClearRenderTargetView(_framebuffer.rtv.Get(), clear_color_with_alpha);
    _context->ClearDepthStencilView(_framebuffer.dsv.Get(), D3D11_CLEAR_DEPTH,
                                     1.0f, 0);
  } else {*/
    const float clear_color_with_alpha[] = {0.0f, 0.0f, 0.0f, 0.0f};
    _context->OMSetRenderTargets(1, &_backbufferRTV, _depthStencilView);

    _context->ClearRenderTargetView(_backbufferRTV, clear_color_with_alpha);
    _context->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH,
                                     1.0f, 0);
  //}
}

void D3D11Renderer::EndDraw() {
  //if (_useToneMappingAndGamma) {
  //  // Resolve multisample framebuffer.
  //  ResolveFrameBuffer(_framebuffer, _resolveFramebuffer,
  //                     DXGI_FORMAT_R16G16B16A16_FLOAT);

  //  // Draw a full screen triangle for postprocessing/tone mapping.
  //  _context->OMSetRenderTargets(1, &_backbufferRTV, nullptr);
  //  _context->IASetInputLayout(nullptr);
  //  _context->VSSetShader(_toneMappingProgram.vertexShader.Get(), nullptr, 0);
  //  _context->PSSetShader(_toneMappingProgram.pixelShader.Get(), nullptr, 0);
  //  _context->PSSetShaderResources(0, 1,
  //                                 _resolveFramebuffer.srv.GetAddressOf());
  //  _context->PSSetSamplers(0, 1, _toneMappingSamplerState.GetAddressOf());
  //  _context->Draw(3, 0);
  //}
}

HRESULT D3D11Renderer::CreateDeviceResources() {
  HRESULT res = S_OK;

  // Determin DirectX hardware feature levels this app will support
  static const D3D_FEATURE_LEVEL featureLevels[] = {
      D3D_FEATURE_LEVEL_11_1, D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0, D3D_FEATURE_LEVEL_9_3,  D3D_FEATURE_LEVEL_9_2,
      D3D_FEATURE_LEVEL_9_1,
  };

  UINT featureLevelCount = 0;
  for (; featureLevelCount < std::size(featureLevels); ++featureLevelCount) {
    if (featureLevels[featureLevelCount] < _minFeatureLevel) break;
  }

  if (featureLevelCount == 0)
    throw std::out_of_range("minFeatureLevel too high!");

  UINT deviceFlags{0};
#ifndef NDEBUG
  deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  res =
      D3D11CreateDevice(nullptr,  // Use the default adapter
                        D3D_DRIVER_TYPE_HARDWARE, 0, deviceFlags, featureLevels,
                        std::size(featureLevels), D3D11_SDK_VERSION, &_device,
                        &_featureLevel, &_context);

  if (FAILED(res)) return res;

  return res;
}

HRESULT D3D11Renderer::CreateDebugLayer() {
  ID3D11Debug* d3dDebug = nullptr;
  if (SUCCEEDED(
          _device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug))) {
    ID3D11InfoQueue* d3dInfoQueue = nullptr;
    if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue),
                                           (void**)&d3dInfoQueue))) {
#ifdef _DEBUG
      d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
      d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif

      D3D11_MESSAGE_ID hide[] = {
          D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
          // Add more message IDs here as needed
      };

      D3D11_INFO_QUEUE_FILTER filter = {};
      filter.DenyList.NumIDs = _countof(hide);
      filter.DenyList.pIDList = hide;
      d3dInfoQueue->AddStorageFilterEntries(&filter);
      d3dInfoQueue->Release();

      return S_OK;
    }
    d3dDebug->Release();
  }

  return S_FALSE;
}

HRESULT D3D11Renderer::CreateFactory() {
#ifdef _DEBUG
  bool debugDXGI = false;
  {
    ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
    if (SUCCEEDED(DXGIGetDebugInterface1(
            0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf())))) {
      debugDXGI = true;

      CHECK(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG,
                               IID_PPV_ARGS(&_dxgiFactory)));

      dxgiInfoQueue->SetBreakOnSeverity(
          DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
      dxgiInfoQueue->SetBreakOnSeverity(
          DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

      DXGI_INFO_QUEUE_MESSAGE_ID hide[] = {
          80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter
                does not control the output on which the swapchain's window
                resides. */
          ,
      };
      DXGI_INFO_QUEUE_FILTER filter = {};
      filter.DenyList.NumIDs = _countof(hide);
      filter.DenyList.pIDList = hide;
      dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
    }
  }

  if (!debugDXGI)
#endif
    CHECK(CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory)));

  return S_OK;
}

HRESULT D3D11Renderer::CreateSwapchain(HWND hWnd, UINT width, UINT height) {
  HRESULT res = S_OK;

  DXGI_SWAP_CHAIN_DESC1 desc{
      .Width = width,
      .Height = height,
      // Swap buffer description
      .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
      // Sample description
      .SampleDesc =
          {
              .Count = 1,  // No multisampling
              .Quality = 0,
          },
      // Swap buffer usage
      .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
      // The number of buffers to use
      .BufferCount = 2,
      .Scaling = DXGI_SCALING_NONE,
      // Swap effect
      .SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
      // Alpha mode
      .AlphaMode = DXGI_ALPHA_MODE_IGNORE,
      //// Allow full screen switching
      //.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
  };

  DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc = {};
  fullScreenDesc.Windowed = TRUE;

  // Create a swapchain
  ComPtr<IDXGIAdapter> adapter;
  res = _dxgiFactory->EnumAdapters(0, &adapter);
  if (FAILED(res)) return res;

  res = _dxgiFactory->CreateSwapChainForHwnd(
      _device, hWnd, &desc,
      &fullScreenDesc,  // A windowed swapchain. For a full-screen swapchain,
                        // configure  DXGI_SWAP_CHAIN_FULLSCREEN_DESC
      NULL,             // No restrict to output
      &_swapchain);

  return res;
}

HRESULT D3D11Renderer::CreateBackBufferView() {
  HRESULT res = S_OK;

  // Create a back buffer
  res = _swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&_backbuffer);
  if (FAILED(res)) return res;

	// Create a render target view
  res = _device->CreateRenderTargetView(_backbuffer, nullptr, &_backbufferRTV);
  if (FAILED(res)) return res;

  // Get the description
  _backbuffer->GetDesc(&_backbufferDesc);

  return res;
}

HRESULT D3D11Renderer::CreateFrameBuffers() {
  HRESULT res = S_OK;

	// Determine maximum supported MSAA level.
  UINT samples;
  for (samples = MAX_SAMPLE; samples > 1; samples /= 2) {
    UINT colorQualityLevels;
    UINT depthStencilQualityLevels;
    _device->CheckMultisampleQualityLevels(DXGI_FORMAT_R16G16B16A16_FLOAT,
                                            samples, &colorQualityLevels);
    _device->CheckMultisampleQualityLevels(
        DXGI_FORMAT_D24_UNORM_S8_UINT, samples, &depthStencilQualityLevels);
    if (colorQualityLevels > 0 && depthStencilQualityLevels > 0) {
      break;
    }
  }

  _framebuffer = CreateFrameBuffer(_backbufferDesc.Width, _backbufferDesc.Height,
                                   samples,
                                   DXGI_FORMAT_R16G16B16A16_FLOAT,
                        DXGI_FORMAT_D24_UNORM_S8_UINT);
  if (samples > 1) {
    _resolveFramebuffer =
        CreateFrameBuffer(_backbufferDesc.Width, _backbufferDesc.Height, 1,
                          DXGI_FORMAT_R16G16B16A16_FLOAT, (DXGI_FORMAT)0);
  } else {
    _resolveFramebuffer = _framebuffer;
  }

  return res;
}

HRESULT D3D11Renderer::CreateToneMappingAndGammaCorrectionProgram() {
  _toneMappingProgram = CreateShaderProgram(
      CompileShaderFromFile(L"shaders/ToneMapping_VS.hlsl", "main", "vs_5_0"),
      CompileShaderFromFile(L"shaders/ToneMapping_PS.hlsl", "main", "ps_5_0"),
      nullptr);

	_toneMappingSamplerState = CreateSamplerState(D3D11_FILTER_MIN_MAG_MIP_LINEAR,
                                                D3D11_TEXTURE_ADDRESS_WRAP);
  return S_OK;
}

HRESULT D3D11Renderer::ConfigureDepthStencilBuffer() {
  HRESULT res = S_OK;

  /* Depth Stencil Buffer */

  // Create the depth stencil buffer
  D3D11_TEXTURE2D_DESC depthBufferDesc{};
  depthBufferDesc.Width = _backbufferDesc.Width;
  depthBufferDesc.Height = _backbufferDesc.Height;
  depthBufferDesc.MipLevels = 1;
  depthBufferDesc.ArraySize = 1;
  depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthBufferDesc.SampleDesc.Count = 1;  // No multisampling
  depthBufferDesc.SampleDesc.Quality = 0;
  depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  depthBufferDesc.CPUAccessFlags = 0;  // No CPU r/w access
  depthBufferDesc.MiscFlags = 0;       // Optional flags none
  res =
      _device->CreateTexture2D(&depthBufferDesc, nullptr, &_depthStencilBuffer);
  if (FAILED(res)) return res;

  /* Depth Stencil State */

  // Depth stencil state description
  D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
  // Set up the depth state
  depthStencilDesc.DepthEnable = true;
  // Turn on writes to the depth-stencil buffer
  depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

  // Set up the stencil state
  depthStencilDesc.StencilEnable = true;
  depthStencilDesc.StencilReadMask = 0xFF;
  depthStencilDesc.StencilWriteMask = 0xFF;

  // Stencil operations if pixel is front-facing
  depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.FrontFace.StencilFunc =
      D3D11_COMPARISON_ALWAYS;  // Always pass the comparison

  // Stencil operations if pixel is back-facing
  depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.BackFace.StencilFunc =
      D3D11_COMPARISON_ALWAYS;  // Always pass the comparison

  // Create the depth stencil state
  _device->CreateDepthStencilState(&depthStencilDesc, &_depthStencilState);

  // Set depth stencil state
  _context->OMSetDepthStencilState(_depthStencilState, 1);

  /* Depth Stencil View */

  // Depth stencil
  D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
  depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  depthStencilViewDesc.Texture2D.MipSlice = 0;

  // Create the depth stencil view
  _device->CreateDepthStencilView(_depthStencilBuffer, &depthStencilViewDesc,
                                  &_depthStencilView);

  return res;
}

HRESULT D3D11Renderer::ConfigureRasterizer() {
  HRESULT res = S_OK;

  // Rasteization state description
  D3D11_RASTERIZER_DESC rasterDesc{};
  rasterDesc.FillMode = D3D11_FILL_SOLID;  // Wireframe or solid
  rasterDesc.CullMode = D3D11_CULL_BACK;   // Back-face culling
  // rasterDesc.FrontCounterClockwise = FALSE;	// Front-face is clockwise
  rasterDesc.DepthBias = 0;         // Depth value added to a given pixel
  rasterDesc.DepthBiasClamp = 0.f;  // Max depth bias of a pixel
  rasterDesc.SlopeScaledDepthBias = 0.f;

  rasterDesc.DepthClipEnable = TRUE;  // Enable clipping based on distance
  rasterDesc.ScissorEnable = FALSE;   // Enable scissor-rectangle culling

  rasterDesc.MultisampleEnable = FALSE;
  rasterDesc.AntialiasedLineEnable = FALSE;

  // Create the rasterizer state
  _device->CreateRasterizerState(&rasterDesc, &_rasterizerState);

  // Set the rasterizer state
  _context->RSSetState(_rasterizerState);

  return res;
}

HRESULT D3D11Renderer::ConfigureViewport() {
  HRESULT res = S_OK;

  // Set the viewport
  _viewport = {};
  _viewport.Width = (float)_backbufferDesc.Width;
  _viewport.Height = (float)_backbufferDesc.Height;
  _viewport.MaxDepth = 1.f;
  _viewport.MinDepth = 0.f;
  _context->RSSetViewports(1, &_viewport);

  return res;
}

MeshBuffer D3D11Renderer::CreateMeshBuffer(
    const std::shared_ptr<Mesh>& mesh) const {
  MeshBuffer buffer = {};
  buffer.stride = sizeof(Vertex);
  buffer.numElements = static_cast<UINT>(mesh->faces().size() * 3);

  const size_t vertexDataSize = mesh->vertices().size() * sizeof(Vertex);
  const size_t indexDataSize = mesh->faces().size() * sizeof(Face);

  {
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = (UINT)vertexDataSize;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA data = {};
    data.pSysMem = &mesh->vertices()[0];
    if (FAILED(_device->CreateBuffer(&desc, &data, &buffer.vertexBuffer))) {
      throw std::runtime_error("Failed to create vertex buffer");
    }
  }
  {
    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = (UINT)indexDataSize;
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

    D3D11_SUBRESOURCE_DATA data = {};
    data.pSysMem = &mesh->faces()[0];
    if (FAILED(_device->CreateBuffer(&desc, &data, &buffer.indexBuffer))) {
      throw std::runtime_error("Failed to create index buffer");
    }
  }
  return buffer;
}

ShaderProgram D3D11Renderer::CreateShaderProgram(
    const std::vector<uint8_t>& vsBytecode,
    const std::vector<uint8_t>& psBytecode,
    const std::vector<D3D11_INPUT_ELEMENT_DESC>* inputLayoutDesc) const {
  ShaderProgram program;

  if (FAILED(_device->CreateVertexShader(vsBytecode.data(), vsBytecode.size(),
                                         nullptr, &program.vertexShader))) {
    throw std::runtime_error(
        "Failed to create vertex shader from compiled bytecode");
  }
  if (FAILED(_device->CreatePixelShader(psBytecode.data(), psBytecode.size(),
                                        nullptr, &program.pixelShader))) {
    throw std::runtime_error(
        "Failed to create pixel shader from compiled bytecode");
  }

  if (inputLayoutDesc) {
    if (FAILED(_device->CreateInputLayout(
            inputLayoutDesc->data(), (UINT)inputLayoutDesc->size(),
            vsBytecode.data(), vsBytecode.size(),
            &program.inputLayout))) {
      throw std::runtime_error("Failed to create shader program input layout");
    }
  }
  return program;
}

ComputeProgram D3D11Renderer::CreateComputeProgram(
    const std::vector<uint8_t>& csBytecode) const {
  ComputeProgram program;
  if (FAILED(_device->CreateComputeShader(csBytecode.data(),
                                           csBytecode.size(), nullptr,
                                           &program.computeShader))) {
    throw std::runtime_error(
        "Failed to create compute shader from compiled bytecode");
  }
  return program;
}

ComPtr<ID3D11SamplerState> D3D11Renderer::CreateSamplerState(
    D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode) const {
  D3D11_SAMPLER_DESC desc = {};
  desc.Filter = filter;
  desc.AddressU = addressMode;
  desc.AddressV = addressMode;
  desc.AddressW = addressMode;
  desc.MaxAnisotropy =
      (filter == D3D11_FILTER_ANISOTROPIC) ? D3D11_REQ_MAXANISOTROPY : 1;
  desc.MinLOD = 0;
  desc.MaxLOD = D3D11_FLOAT32_MAX;

  ComPtr<ID3D11SamplerState> samplerState;
  if (FAILED(_device->CreateSamplerState(&desc, &samplerState))) {
    throw std::runtime_error("Failed to create sampler state");
  }
  return samplerState;
}

Texture D3D11Renderer::CreateTexture(UINT width, UINT height,
                                     DXGI_FORMAT format, UINT levels) const {
  Texture texture;
  texture.width = width;
  texture.height = height;
  texture.levels =
      (levels > 0) ? levels : Utility::numMipmapLevels(width, height);

  D3D11_TEXTURE2D_DESC desc = {};
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = levels;
  desc.ArraySize = 1;
  desc.Format = format;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
  if (levels == 0) {
    desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
    desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
  }

  if (FAILED(_device->CreateTexture2D(&desc, nullptr, &texture.texture))) {
    ErrorExit(L"CreateTexture2D");
    throw std::runtime_error("Failed to create 2D texture");
  }

  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Format = desc.Format;
  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MostDetailedMip = 0;
  srvDesc.Texture2D.MipLevels = -1;
  if (FAILED(_device->CreateShaderResourceView(texture.texture.Get(), &srvDesc,
                                                &texture.srv))) {
    ErrorExit(L"CreateTexture2D");
    throw std::runtime_error("Failed to create 2D texture SRV");
  }
  return texture;
}

Texture D3D11Renderer::CreateTexture(const std::shared_ptr<class Image>& image,
                                     DXGI_FORMAT format, UINT levels) const {
  Texture texture =
      CreateTexture(image->width(), image->height(), format, levels);
  _context->UpdateSubresource(texture.texture.Get(), 0, nullptr,
                               image->pixels<void>(), image->pitch(), 0);
  if (levels == 0) {
    _context->GenerateMips(texture.srv.Get());
  }
  return texture;
}

Texture D3D11Renderer::CreateTextureCube(UINT width, UINT height,
                                         DXGI_FORMAT format,
                                         UINT levels) const {
  Texture texture;
  texture.width = width;
  texture.height = height;
  texture.levels =
      (levels > 0) ? levels : Utility::numMipmapLevels(width, height);

  D3D11_TEXTURE2D_DESC desc = {};
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = levels;
  desc.ArraySize = 6;
  desc.Format = format;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
  desc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
  if (levels == 0) {
    desc.BindFlags |= D3D11_BIND_RENDER_TARGET;
    desc.MiscFlags |= D3D11_RESOURCE_MISC_GENERATE_MIPS;
  }

  if (FAILED(_device->CreateTexture2D(&desc, nullptr, &texture.texture))) {
    throw std::runtime_error("Failed to create cubemap texture");
  }

  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Format = desc.Format;
  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
  srvDesc.TextureCube.MostDetailedMip = 0;
  srvDesc.TextureCube.MipLevels = -1;
  if (FAILED(_device->CreateShaderResourceView(texture.texture.Get(), &srvDesc,
                                                &texture.srv))) {
    throw std::runtime_error("Failed to create cubemap texture SRV");
  }
  return texture;
}

Texture D3D11Renderer::CreateTextureCube(
    const std::shared_ptr<class Image>& image, DXGI_FORMAT format,
    UINT levels) const {
  Texture texture =
      CreateTextureCube(image->width(), image->height(), format, levels);
  if (levels == 0) {
    _context->GenerateMips(texture.srv.Get());
  }
  return texture;
}

void D3D11Renderer::CreateTextureUAV(Texture& texture, UINT mipSlice) const {
  assert(texture.texture);

  D3D11_TEXTURE2D_DESC desc;
  texture.texture->GetDesc(&desc);

  D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
  uavDesc.Format = desc.Format;
  if (desc.ArraySize == 1) {
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = mipSlice;
  } else {
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
    uavDesc.Texture2DArray.MipSlice = mipSlice;
    uavDesc.Texture2DArray.FirstArraySlice = 0;
    uavDesc.Texture2DArray.ArraySize = desc.ArraySize;
  }

  if (FAILED(_device->CreateUnorderedAccessView(texture.texture.Get(),
                                                 &uavDesc, &texture.uav))) {
    throw std::runtime_error("Failed to create texture UAV");
  }
}

FrameBuffer D3D11Renderer::CreateFrameBuffer(
    UINT width, UINT height, UINT samples, DXGI_FORMAT colorFormat,
    DXGI_FORMAT depthstencilFormat) const {
  FrameBuffer fb;
  fb.width = width;
  fb.height = height;
  fb.samples = samples;

  D3D11_TEXTURE2D_DESC desc = {};
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.SampleDesc.Count = samples;

  if (colorFormat != DXGI_FORMAT_UNKNOWN) {
    desc.Format = colorFormat;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET;
    if (samples <= 1) {
      desc.BindFlags |= D3D11_BIND_SHADER_RESOURCE;
    }
    if (FAILED(_device->CreateTexture2D(&desc, nullptr, &fb.colorTexture))) {
      throw std::runtime_error("Failed to create FrameBuffer color texture");
    }

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
    rtvDesc.Format = desc.Format;
    rtvDesc.ViewDimension = (samples > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMS
                                          : D3D11_RTV_DIMENSION_TEXTURE2D;
    if (FAILED(_device->CreateRenderTargetView(fb.colorTexture.Get(), &rtvDesc,
                                                &fb.rtv))) {
      throw std::runtime_error(
          "Failed to create FrameBuffer render target view");
    }

    if (samples <= 1) {
      D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
      srvDesc.Format = desc.Format;
      srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
      srvDesc.Texture2D.MostDetailedMip = 0;
      srvDesc.Texture2D.MipLevels = 1;
      if (FAILED(_device->CreateShaderResourceView(fb.colorTexture.Get(),
                                                    &srvDesc, &fb.srv))) {
        throw std::runtime_error(
            "Failed to create FrameBuffer shader resource view");
      }
    }
  }

  if (depthstencilFormat != DXGI_FORMAT_UNKNOWN) {
    desc.Format = depthstencilFormat;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    if (FAILED(_device->CreateTexture2D(&desc, nullptr,
                                         &fb.depthStencilTexture))) {
      throw std::runtime_error(
          "Failed to create FrameBuffer depth-stencil texture");
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = desc.Format;
    dsvDesc.ViewDimension = (samples > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS
                                          : D3D11_DSV_DIMENSION_TEXTURE2D;
    if (FAILED(_device->CreateDepthStencilView(fb.depthStencilTexture.Get(),
                                                &dsvDesc, &fb.dsv))) {
      throw std::runtime_error(
          "Failed to create FrameBuffer depth-stencil view");
    }
  }

  return fb;
}

void D3D11Renderer::ResolveFrameBuffer(const FrameBuffer& srcfb,
                                       const FrameBuffer& dstfb,
                                       DXGI_FORMAT format) const {
  if (srcfb.colorTexture != dstfb.colorTexture) {
    _context->ResolveSubresource(dstfb.colorTexture.Get(), 0,
                                  srcfb.colorTexture.Get(), 0, format);
  }
}

ComPtr<ID3D11Buffer> D3D11Renderer::CreateConstantBuffer(const void* data,
                                                         UINT size) const {
  D3D11_BUFFER_DESC desc = {};
  desc.ByteWidth = static_cast<UINT>(size);
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

  D3D11_SUBRESOURCE_DATA bufferData = {};
  bufferData.pSysMem = data;

  ComPtr<ID3D11Buffer> buffer;
  const D3D11_SUBRESOURCE_DATA* bufferDataPtr = data ? &bufferData : nullptr;
  if (FAILED(_device->CreateBuffer(&desc, bufferDataPtr, &buffer))) {
    throw std::runtime_error("Failed to create constant buffer");
  }
  return buffer;
}

void D3D11Renderer::CopyDataToDeviceBuffer(ComPtr<ID3D11Buffer>& buffer,
                                           const void* data) {
  _context->UpdateSubresource(buffer.Get(), 0, nullptr, data, 0, 0);
}

ComPtr<ID3DBlob> D3D11Renderer::CompileShader(const std::string& filename,
                                              const std::string& entryPoint,
                                              const std::string& profile) {
  UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
  flags |= D3DCOMPILE_DEBUG;
  flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

  ComPtr<ID3DBlob> shader;
  ComPtr<ID3DBlob> errorBlob;

  std::printf("Compiling HLSL shader: %s [%s]\n", filename.c_str(),
              entryPoint.c_str());

  if (FAILED(D3DCompileFromFile(Utility::convertToUTF16(filename).c_str(),
                                nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                entryPoint.c_str(), profile.c_str(), flags, 0,
                                &shader, &errorBlob))) {
    ErrorExit(L"D3DCompileFromFile");

    std::string errorMsg = "Shader compilation failed: " + filename;
    if (errorBlob) {
      errorMsg += std::string("\n") +
                  static_cast<const char*>(errorBlob->GetBufferPointer());
    }
    throw std::runtime_error(errorMsg);
  }
  return shader;
}
