#include "demo_app.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include "D3DEngine/Renderer/D3D11Renderer.h"
#include "D3DEngine/WinApp/WinApp.h"
#include "camera.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

#define USE_FLIPMODE 1   // In order to not show warnings, use flip mode
#define VSYNC_ENABLED 0  // diable v-sync when 0, otherwise v-sync is on
#define USE_GUI 1
#define USE_CAM 0

DemoApp* loadedApp{nullptr};

void DemoApp::Initialize() {
  if (loadedApp) abort();

  ///////////////////////// DO NOT MODIFY /////////////////////////
  // 윈도우 생성
  WinApp::App_Init();
  // TODO: 윈도우 타이틀이랑 스타일 추가할 것.
  WindowStyleFlags styleFlags = WS_OVERLAPPED;
  hWindow =
      WinApp::App_CreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, TITLE, styleFlags);

  // GameEngine의 이니셜라이제이션
  Super::Initialize();
  //
  /////////////////////////////////////////////////////////////////

  /////////////////////////////////////////////////////////////////
  // Other initialization stages
  //
  //
  //

  _renderer = new D3D11Renderer();
  if (FAILED(_renderer->Initialize(hWindow, SCREEN_WIDTH, SCREEN_HEIGHT)))
    throw std::runtime_error("Initialization of D3D 11 failed!");

  InitTransformMatrices();
  InitCamera();
  InitBuffers();
  InitShaders();
  InitTextures();
  InitSamplers();
  InitSpecularBRDF_LUT();
  
	InitIBL();

	InitMeshes();
  InitLights();
#if USE_GUI == 1
  InitImgui();
#endif

  // 초기화 True
  isInitialized = true;
  loadedApp = this;
}

void DemoApp::Execute() {
  // 게임 루프 실행
  Super::Execute();
}

void DemoApp::Shutdown() {
  if (isInitialized) {
    /////////////////////////////////////////////////////////////////
    // Other finalization stages
    //
    //
    //
    _renderer->Shutdown();

    ///////////////////////// DO NOT MODIFY /////////////////////////
    // 게임 엔진 셧다운
    Super::Shutdown();

    // 윈도우 파괴
    WinApp::App_Destroy();
    //
    /////////////////////////////////////////////////////////////////
  }

  loadedApp = nullptr;
}

void DemoApp::FixedUpdate(float dt) {}

void DemoApp::Update(float dt) {
  static bool started = false;
#ifndef NDEBUG
  frameTime += dt;
#endif

#if USE_CAM == 1
  _camera->Update(dt);
#endif
}

void DemoApp::Render() {
#ifndef NDEBUG
  // FPS
  if (frameTime >= 1.0f) {
    frameTime -= 1.0f;
    fps = count;
    count = 0;
  }
#endif

  _renderer->BeginDraw();

	_renderer->_context->IASetPrimitiveTopology(
      D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

#if USE_CAM == 1
  _view = _camera->GetViewTransform();
#endif

  _transformConstants.viewProj = _view * _proj;
  _transformConstants.sceneRotation = XMMatrixIdentity();
  _renderer->CopyDataToDeviceBuffer(_cboTransform, &_transformConstants);
	_renderer->CopyDataToDeviceBuffer(_cboShading, &_shadingConstants);

	// Draw skybox.
  if (_useIBL) {
    _renderer->_context->IASetInputLayout(
        _skyboxProgram.inputLayout.Get());
    _renderer->_context->IASetVertexBuffers(
        0, 1, _skybox.vertexBuffer.GetAddressOf(), &_skybox.stride,
        &_skybox.offset);
    _renderer->_context->IASetIndexBuffer(_skybox.indexBuffer.Get(),
                                          DXGI_FORMAT_R32_UINT, 0);
    _renderer->_context->VSSetShader(_skyboxProgram.vertexShader.Get(),
                                      nullptr, 0);
    _renderer->_context->PSSetShader(_skyboxProgram.pixelShader.Get(),
                                      nullptr, 0);
    _renderer->_context->PSSetShaderResources(
        0, 1, _envTexture.srv.GetAddressOf());
    _renderer->_context->PSSetSamplers(0, 1,
                                        _defaultSampler.GetAddressOf());
    _renderer->_context->OMSetDepthStencilState(
        _skyboxDepthStencilState.Get(), 0);
    _renderer->_context->DrawIndexed(_skybox.numElements, 0, 0);
  }

	// Draw PBR model.
  ID3D11ShaderResourceView* const pbrModelSRVs[] = {
      _albedoTexture.srv.Get(),    _normalTexture.srv.Get(),
      _metalnessTexture.srv.Get(), _roughnessTexture.srv.Get(),
      _envTexture.srv.Get(),       _irmapTexture.srv.Get(),
      _spBRDF_LUT.srv.Get(),
  };
  ID3D11SamplerState* const pbrModelSamplers[] = {
      _defaultSampler.Get(),
      _spBRDF_Sampler.Get(),
  };

	_renderer->_context->IASetInputLayout(_pbrProgram.inputLayout.Get());
  _renderer->_context->IASetVertexBuffers(0, 1, _pbrModel.vertexBuffer.GetAddressOf(),
                                &_pbrModel.stride, &_pbrModel.offset);
  _renderer->_context->IASetIndexBuffer(_pbrModel.indexBuffer.Get(),
                              DXGI_FORMAT_R32_UINT, 0);
  _renderer->_context->VSSetShader(_pbrProgram.vertexShader.Get(), nullptr, 0);
  _renderer->_context->PSSetShader(_pbrProgram.pixelShader.Get(), nullptr, 0);
  _renderer->_context->PSSetShaderResources(0, 7, pbrModelSRVs);
  _renderer->_context->PSSetSamplers(0, 2, pbrModelSamplers);
  _renderer->_context->OMSetDepthStencilState(_defaultDepthStencilState.Get(), 0);
  _renderer->_context->DrawIndexed(_pbrModel.numElements, 0, 0);

	_renderer->EndDraw();

#if USE_GUI == 1
  // Start the Dear ImGui frame
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  if (ImGui::Begin("Properties")) {
    ImGui::Text("Gamma: ");

  }
  ImGui::End();

  // Rendering
  ImGui::Render();
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif

	_renderer->_swapchain->Present(1, 0);

#ifndef NDEBUG
  ++count;
#endif
}

void DemoApp::InitTransformMatrices() {
  // Setup the vertical field of view
  float vfov = PI_F / 2.f;  // 90 degree field of view
  // Get the screen aspect ratio
  float aspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;

  // Create the projection matrix
  _proj = XMMatrixPerspectiveFovLH(vfov, aspectRatio, 0.01f, 10000.f);
  // Create the Orthographic projection matrix
  //_proj = XMMatrixOrthographicLH((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT,
  //0.01f, 100.f);
}

void DemoApp::InitCamera() {
#if USE_CAM == 1
  _camera = new Camera(GetModuleHandle(NULL), hWindow);
  _view = _camera->GetViewTransform();
#else
  XMVECTOR eye = g_camPos;
  XMVECTOR viewDir{ 0.f, 0.f, -1.f };
  XMVECTOR upDir{0.f, 1.f, 0.f};
  _view = XMMatrixLookToLH(eye, viewDir, upDir);
  //_view = XMMatrixLookAtLH({10, 0, 10, 0}, {0, 0, 0, 0}, upDir);
#endif
}

void DemoApp::InitBuffers() {
  _cboTransform = _renderer->CreateConstantBuffer<cbTransformConstants>();
  _cboShading = _renderer->CreateConstantBuffer<cbShadingConstants>();
}

void DemoApp::InitShaders() {
  const std::vector<D3D11_INPUT_ELEMENT_DESC> meshInputLayout = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
  };

  const std::vector<D3D11_INPUT_ELEMENT_DESC> skyboxInputLayout = {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
       D3D11_INPUT_PER_VERTEX_DATA, 0},
  };

  _pbrProgram = _renderer->CreateShaderProgram(
      CompileShaderFromFile(L"shaders/PBR_VS.hlsl", "main", "vs_5_0"),
      CompileShaderFromFile(L"shaders/PBR_PS.hlsl", "main", "ps_5_0"),
      &meshInputLayout);

	_skyboxProgram = _renderer->CreateShaderProgram(
      CompileShaderFromFile(L"shaders/SkyBox_VS.hlsl", "main", "vs_5_0"),
      CompileShaderFromFile(L"shaders/SkyBox_PS.hlsl", "main", "ps_5_0"),
      &skyboxInputLayout);
}

void DemoApp::InitTextures() {
  _albedoTexture =
      _renderer->CreateTexture(Image::fromFile("assets/textures/cerberus_A.png"),
                                  DXGI_FORMAT_R8G8B8A8_UNORM_SRGB);
  _normalTexture = _renderer->CreateTexture(
      Image::fromFile("assets/textures/cerberus_N.png"),
                                  DXGI_FORMAT_R8G8B8A8_UNORM);
  _metalnessTexture = _renderer->CreateTexture(
      Image::fromFile("assets/textures/cerberus_M.png", 1), DXGI_FORMAT_R8_UNORM);
  _roughnessTexture = _renderer->CreateTexture(
      Image::fromFile("assets/textures/cerberus_R.png", 1), DXGI_FORMAT_R8_UNORM);

}

void DemoApp::InitSamplers() {
  _defaultSampler = _renderer->CreateSamplerState(D3D11_FILTER_ANISOTROPIC,
                                                  D3D11_TEXTURE_ADDRESS_WRAP);
  _computeSampler = _renderer->CreateSamplerState(
      D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP);
}

void DemoApp::InitSpecularBRDF_LUT() {
  ComputeProgram spBRDFProgram = _renderer->CreateComputeProgram(
      CompileShaderFromFile(L"shaders/SpecularBRDF_LUT_CS.hlsl", "main", "cs_5_0"));

  _spBRDF_LUT = _renderer->CreateTexture(256, 256, DXGI_FORMAT_R16G16_FLOAT, 1);
  _spBRDF_Sampler = _renderer->CreateSamplerState(D3D11_FILTER_MIN_MAG_MIP_LINEAR,
                                        D3D11_TEXTURE_ADDRESS_CLAMP);
  _renderer->CreateTextureUAV(_spBRDF_LUT, 0);

  _renderer->_context->CSSetUnorderedAccessViews(
      0, 1, _spBRDF_LUT.uav.GetAddressOf(), nullptr);
  _renderer->_context->CSSetShader(spBRDFProgram.computeShader.Get(), nullptr, 0);
  _renderer->_context->Dispatch(_spBRDF_LUT.width / 32, _spBRDF_LUT.height / 32,
                              1);
  _renderer->_context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
}

void DemoApp::InitIBL() {
  // Unfiltered environment cube map (temporary).
  Texture envTextureUnfiltered =
      _renderer->CreateTextureCube(1024, 1024, DXGI_FORMAT_R16G16B16A16_FLOAT);
  _renderer->CreateTextureUAV(envTextureUnfiltered, 0);

  // Load & convert equirectangular environment map to a cubemap texture.
  {
    ComputeProgram equirectToCubeProgram = _renderer->CreateComputeProgram(
        CompileShaderFromFile(L"shaders/Equirect2Cube_CS.hlsl", "main", "cs_5_0"));
    Texture envTextureEquirect = _renderer->CreateTexture(
        Image::fromFile("assets/textures/environment.hdr"), DXGI_FORMAT_R32G32B32A32_FLOAT, 1);

    _renderer->_context->CSSetShaderResources(
        0, 1,
                                    envTextureEquirect.srv.GetAddressOf());
    _renderer->_context->CSSetUnorderedAccessViews(
        0, 1, envTextureUnfiltered.uav.GetAddressOf(), nullptr);
    _renderer->_context->CSSetSamplers(0, 1, _computeSampler.GetAddressOf());
    _renderer->_context->CSSetShader(equirectToCubeProgram.computeShader.Get(),
                                   nullptr, 0);
    _renderer->_context->Dispatch(envTextureUnfiltered.width / 32,
                                envTextureUnfiltered.height / 32, 6);
    _renderer->_context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
  }

  _renderer->_context->GenerateMips(envTextureUnfiltered.srv.Get());

  // Compute pre-filtered specular environment map.
  {
    struct SpecularMapFilterSettingsCB {
      float roughness;
      float padding[3];
    };
    ComputeProgram spmapProgram =
        _renderer->CreateComputeProgram(
        CompileShaderFromFile(L"shaders/SpecularIBLMap_CS.hlsl", "main", "cs_5_0"));
    ComPtr<ID3D11Buffer> spmapCB =
        _renderer->CreateConstantBuffer<SpecularMapFilterSettingsCB>();

    _envTexture =
        _renderer->CreateTextureCube(1024, 1024, DXGI_FORMAT_R16G16B16A16_FLOAT);

    // Copy 0th mipmap level into destination environment map.
    for (int arraySlice = 0; arraySlice < 6; ++arraySlice) {
      const UINT subresourceIndex =
          D3D11CalcSubresource(0, arraySlice, _envTexture.levels);
      _renderer->_context->CopySubresourceRegion(
          _envTexture.texture.Get(), subresourceIndex, 0, 0, 0,
          envTextureUnfiltered.texture.Get(), subresourceIndex, nullptr);
    }

    _renderer->_context->CSSetShaderResources(
        0, 1, envTextureUnfiltered.srv.GetAddressOf());
    _renderer->_context->CSSetSamplers(0, 1, _computeSampler.GetAddressOf());
    _renderer->_context->CSSetShader(spmapProgram.computeShader.Get(), nullptr,
                                   0);

    // Pre-filter rest of the mip chain.
    const float deltaRoughness =
        1.0f / std::max(float(_envTexture.levels - 1), 1.0f);
    for (UINT level = 1, size = 512; level < _envTexture.levels;
         ++level, size /= 2) {
      const UINT numGroups = std::max<UINT>(1, size / 32);
      _renderer->CreateTextureUAV(_envTexture, level);

      const SpecularMapFilterSettingsCB spmapConstants = {level *
                                                          deltaRoughness};
      _renderer->_context->UpdateSubresource(spmapCB.Get(), 0, nullptr,
                                             &spmapConstants, 0, 0);

      _renderer->_context->CSSetConstantBuffers(0, 1, spmapCB.GetAddressOf());
      _renderer->_context->CSSetUnorderedAccessViews(
          0, 1, _envTexture.uav.GetAddressOf(), nullptr);
      _renderer->_context->Dispatch(numGroups, numGroups, 6);
    }
    _renderer->_context->CSSetConstantBuffers(0, 1, nullBuffer);
    _renderer->_context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
  }

	// Compute diffuse irradiance cubemap.
  ComputeProgram irmapProgram = _renderer->CreateComputeProgram(
      CompileShaderFromFile(L"shaders/DiffuseIrradiance_CS.hlsl", "main", "cs_5_0"));

  _irmapTexture =
      _renderer->CreateTextureCube(32, 32, DXGI_FORMAT_R16G16B16A16_FLOAT, 1);
  _renderer->CreateTextureUAV(_irmapTexture, 0);

  _renderer->_context->CSSetShaderResources(0, 1,
                                            _envTexture.srv.GetAddressOf());
  _renderer->_context->CSSetSamplers(0, 1, _computeSampler.GetAddressOf());
  _renderer->_context->CSSetUnorderedAccessViews(
      0, 1, _irmapTexture.uav.GetAddressOf(),
                                       nullptr);
  _renderer->_context->CSSetShader(irmapProgram.computeShader.Get(), nullptr,
                                   0);
  _renderer->_context->Dispatch(_irmapTexture.width / 32,
                                _irmapTexture.height / 32, 6);
  _renderer->_context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);

}

void DemoApp::InitMeshes() {
  _pbrModel = _renderer->CreateMeshBuffer(Mesh::fromFile("assets/model/cerberus.fbx"));
  _skybox = _renderer->CreateMeshBuffer(Mesh::fromFile("assets/model/skybox.obj"));
}

void DemoApp::InitLights() {
  // Update shading constant buffer (for pixel shader).
  _shadingConstants.eyePosition = g_camPos;
  _shadingConstants.useIBL = _useIBL;
  _shadingConstants.gamma = 1.f;

	Light light1, light2, light3;
  light1.direction = {-1.0f,  0.0f, 0.0f, 0.f};
  light2.direction = { 1.0f,  0.0f, 0.0f, 0.f};
  light3.direction = { 0.0f, -1.0f, 0.0f, 0.f};

	light1.radiance = {1.0f, 1.0f, 1.0f, 1.f};
  light2.radiance = {1.0f, 1.0f, 1.0f, 1.f};
  light3.radiance = {1.0f, 1.0f, 1.0f, 1.f};

	light1.enabled = true;

	_shadingConstants.lights[0] = light1;
	_shadingConstants.lights[1] = light2;
	_shadingConstants.lights[2] = light3;
}

void DemoApp::InitImgui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO& io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer backends
  ImGui_ImplWin32_Init(hWindow);
  ImGui_ImplDX11_Init(_renderer->_device, _renderer->_context);
}
