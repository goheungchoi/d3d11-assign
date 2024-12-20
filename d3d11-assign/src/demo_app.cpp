#include "demo_app.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include "D3DEngine/Renderer/D3D11Renderer.h"
#include "D3DEngine/WinApp/WinApp.h"
#include "camera.h"
#include <dinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

#include <d3d9helper.h>

#define USE_FLIPMODE 1   // In order to not show warnings, use flip mode
#define VSYNC_ENABLED 0  // diable v-sync when 0, otherwise v-sync is on
#define USE_GUI 1

DemoApp* loadedApp{nullptr};

XMVECTOR g_eyePos{};

bool _enableCamera{false};
IDirectInputDevice8* DIKeyboard;
IDirectInputDevice8* DIMouse;
DIMOUSESTATE mouseLastState{};
LPDIRECTINPUT8 DirectInput{};

bool gammaToggle{false};

float elapsedTime{0.f};

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

	CHECK(DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION,IID_IDirectInput8,(void**)&DirectInput, NULL));

	CHECK(DirectInput->CreateDevice(GUID_SysKeyboard, &DIKeyboard, NULL));
	CHECK(DirectInput->CreateDevice(GUID_SysMouse, &DIMouse, NULL));

	CHECK(DIKeyboard->SetDataFormat(&c_dfDIKeyboard));
  CHECK(DIKeyboard->SetCooperativeLevel(hWindow,
                                        DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));

	CHECK(DIMouse->SetDataFormat(&c_dfDIMouse));
  CHECK(DIMouse->SetCooperativeLevel(
      hWindow, DISCL_NONEXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND));


  InitTransformMatrices();
  InitCamera();
  InitBuffers();
  InitShaders();
  InitTextures();
  InitSamplers();

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

void DemoApp::FixedUpdate(float dt) {
  DIMOUSESTATE mouseCurrState;
  BYTE keyboardState[256];

	DIMouse->Acquire();
  DIKeyboard->Acquire();
  
	DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);
  DIKeyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);

  if (keyboardState[DIK_ESCAPE] & 0x80) 
		PostMessage(hWindow, WM_DESTROY, 0, 0);

  if (mouseCurrState.rgbButtons[0] & 0x80) {
    while (ShowCursor(false) >= 0);

    if (keyboardState[DIK_Q] & 0x80) {
      _camera->MoveDownUp(-dt);
    }
    if (keyboardState[DIK_E] & 0x80) {
      _camera->MoveDownUp(dt);
    }
    if (keyboardState[DIK_A] & 0x80) {
      _camera->MoveLeftRight(-dt);
    }
    if (keyboardState[DIK_D] & 0x80) {
      _camera->MoveLeftRight(dt);
    }
    if (keyboardState[DIK_W] & 0x80) {
      _camera->MoveBackForward(dt);
    }
    if (keyboardState[DIK_S] & 0x80) {
      _camera->MoveBackForward(-dt);
    }
    if ((mouseCurrState.lX != mouseLastState.lX) ||
        (mouseCurrState.lY != mouseLastState.lY)) {
      _camera->RotateAroundXAxis(mouseCurrState.lY * 0.1f);
      _camera->RotateAroundYAxis(mouseCurrState.lX * 0.1f);
      mouseLastState = mouseCurrState;
    }

		RECT rect;
    GetClientRect(hWindow, &rect);
    MapWindowPoints(hWindow, nullptr, (POINT*)(&rect), 2);
    ClipCursor(&rect);
  } else {
    while (ShowCursor(true) < 0);
    ClipCursor(nullptr);
		mouseLastState = {};
	}
	
	elapsedTime += dt;
  if (elapsedTime > 0.1) {
    if (keyboardState[DIK_F1] & 0x80) {
      _shadingConstants.lights[0].enabled =
          !_shadingConstants.lights[0].enabled;
    }
    if (keyboardState[DIK_F2] & 0x80) {
      _shadingConstants.lights[1].enabled =
          !_shadingConstants.lights[1].enabled;
    }
    if (keyboardState[DIK_F3] & 0x80) {
      _shadingConstants.lights[2].enabled =
          !_shadingConstants.lights[2].enabled;
    }
    if (keyboardState[DIK_F4] & 0x80) {
      _shadingConstants.useIBL = !_shadingConstants.useIBL;
    }
    if (keyboardState[DIK_F5] & 0x80) {
      gammaToggle = !gammaToggle;
      if (gammaToggle)
        _shadingConstants.gamma = 2.2;
      else
        _shadingConstants.gamma = 1.0;
    }
    elapsedTime = 0.f;
  }

}

void DemoApp::Update(float dt) {
  static bool started = false;
#ifndef NDEBUG
  frameTime += dt;
#endif

  g_camPos.m128_f32[2] = g_camDist;
  XMVECTOR eye = g_camPos;
  XMVECTOR viewDir{0.f, 0.f, -1.f};
  XMVECTOR upDir{0.f, 1.f, 0.f};
  // _view = XMMatrixLookToLH(eye, viewDir, upDir);
  XMMATRIX transform = XMMatrixTranslationFromVector(g_camPos) *
                       XMMatrixRotationY(XMConvertToRadians(rotation));
  XMVECTOR dump1, dump2;
  XMMatrixDecompose(&dump1, &dump2, &eye, transform);
  g_eyePos = eye;
  _view = XMMatrixLookAtLH(eye, {0, 0, 0, 0}, upDir);
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
  _renderer->_context->RSSetState(_defaultRasterizerState.Get());

  _view = _camera->GetViewTransform();

  _transformConstants.view = XMMatrixTranspose(_view);
  _transformConstants.proj = XMMatrixTranspose(_proj);
  _transformConstants.sceneRotation =
      XMMatrixTranspose(XMMatrixScaling(0.8, 0.8, 0.8));
  _renderer->CopyDataToDeviceBuffer(_cboTransform, &_transformConstants);

  _shadingConstants.eyePosition = _camera->GetPosition();

  _renderer->CopyDataToDeviceBuffer(_cboShading, &_shadingConstants);

  _renderer->_context->VSSetConstantBuffers(0, 1, _cboTransform.GetAddressOf());
  _renderer->_context->PSSetConstantBuffers(0, 1, _cboShading.GetAddressOf());

  // Draw skybox.
  if (_shadingConstants.useIBL) {
    _renderer->_context->IASetInputLayout(_skyboxProgram.inputLayout.Get());
    _renderer->_context->IASetVertexBuffers(0, 1,
                                            _skybox.vertexBuffer.GetAddressOf(),
                                            &_skybox.stride, &_skybox.offset);
    _renderer->_context->IASetIndexBuffer(_skybox.indexBuffer.Get(),
                                          DXGI_FORMAT_R32_UINT, 0);
    _renderer->_context->VSSetShader(_skyboxProgram.vertexShader.Get(), nullptr,
                                     0);
    _renderer->_context->PSSetShader(_skyboxProgram.pixelShader.Get(), nullptr,
                                     0);
    _renderer->_context->PSSetShaderResources(0, 1, _environmentMap.srv.GetAddressOf());
    _renderer->_context->PSSetSamplers(0, 1, _defaultSampler.GetAddressOf());
    _renderer->_context->OMSetDepthStencilState(_skyboxDepthStencilState.Get(),
                                                0);
    _renderer->_context->DrawIndexed(_skybox.numElements, 0, 0);
  }

  // Draw PBR model.
  ID3D11ShaderResourceView* const pbrModelSRVs[] = {
      _albedoTexture.srv.Get(),    _normalTexture.srv.Get(),
      _metalnessTexture.srv.Get(), _roughnessTexture.srv.Get(),
      _specularTexture.srv.Get(),  _irradianceTexture.srv.Get(),
      _specularBRDF_LUT.srv.Get(),
  };
  ID3D11SamplerState* const pbrModelSamplers[] = {
      _defaultSampler.Get(),
      _spBRDF_Sampler.Get(),
  };

  _renderer->_context->IASetInputLayout(_pbrProgram.inputLayout.Get());
  _renderer->_context->IASetVertexBuffers(0, 1,
                                          _pbrModel.vertexBuffer.GetAddressOf(),
                                          &_pbrModel.stride, &_pbrModel.offset);
  _renderer->_context->IASetIndexBuffer(_pbrModel.indexBuffer.Get(),
                                        DXGI_FORMAT_R32_UINT, 0);
  _renderer->_context->VSSetShader(_pbrProgram.vertexShader.Get(), nullptr, 0);
  _renderer->_context->PSSetShader(_pbrProgram.pixelShader.Get(), nullptr, 0);
  _renderer->_context->PSSetShaderResources(0, 7, pbrModelSRVs);
  _renderer->_context->PSSetSamplers(0, 2, pbrModelSamplers);
  _renderer->_context->OMSetDepthStencilState(_defaultDepthStencilState.Get(),
                                              0);
  _renderer->_context->DrawIndexed(_pbrModel.numElements, 0, 0);

  _renderer->EndDraw();

#if USE_GUI == 1
  // Start the Dear ImGui frame
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  if (ImGui::Begin("Properties")) {
    ImGui::Text("Metalness: ");
    ImGui::SliderFloat("Metalness", &_shadingConstants.g_metalness, 0.f, 1.f);
    ImGui::Text("Roughness: ");
    ImGui::SliderFloat("Roughness", &_shadingConstants.g_roughness, 0.f, 1.f);
    ImGui::Text("Gamma: ");
    ImGui::SliderFloat("Gamma value", &_shadingConstants.gamma, 0.0, 5.0);
    ImGui::Text("Use IBL: ");
    ImGui::Checkbox("UseIBL", (bool*)&_shadingConstants.useIBL);
    ImGui::Text("Light 1 Properties: ");
    ImGui::Checkbox("Enable Light 1", (bool*)&_shadingConstants.lights[0].enabled);
    ImGui::SliderFloat3("Light 1 Radiance",
                        (float*) & (_shadingConstants.lights[0].radiance), 0.f, 10.f);
		ImGui::Text("Light 2 Properties: ");
    ImGui::Checkbox("Enable Light 2", (bool*)&_shadingConstants.lights[1].enabled);
    ImGui::SliderFloat3("Light 2 Radiance",
                        (float*)&(_shadingConstants.lights[1].radiance), 0.f,
                        10.f);
    ImGui::Text("Light 3 Properties: ");
    ImGui::Checkbox("Enable Light 3", (bool*)&_shadingConstants.lights[2].enabled);
    ImGui::SliderFloat3("Light 3 Radiance",
                        (float*)&(_shadingConstants.lights[2].radiance), 0.f,
                        10.f);
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
  // 0.01f, 100.f);
}

void DemoApp::InitCamera() {
  _camera = new Camera(GetModuleHandle(NULL), hWindow);
  _view = _camera->GetViewTransform();
  /*XMVECTOR eye = g_camPos;
  XMVECTOR viewDir{ 0.f, 0.f, -1.f };
  XMVECTOR upDir{0.f, 1.f, 0.f};
  _view = XMMatrixLookToLH(eye, viewDir, upDir)*/
  ;
#if USE_CAM == 1
  _view = XMMatrixLookAtLH(g_camPos, {0, 0, 0, 0}, {0.f, 1.f, 0.f});
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
  _environmentMap = _renderer->CreateTextureCube("assets/textures/BakerEnv.dds",
      DXGI_FORMAT_R32G32B32A32_FLOAT, 10);

  _albedoTexture = _renderer->CreateTexture(
      Image::fromFile("assets/textures/cerberus_A.png"),
      DXGI_FORMAT_R8G8B8A8_UNORM);
  _normalTexture = _renderer->CreateTexture(
      Image::fromFile("assets/textures/cerberus_N.png"),
      DXGI_FORMAT_R8G8B8A8_UNORM);
  _metalnessTexture = _renderer->CreateTexture(
      Image::fromFile("assets/textures/cerberus_M.png", 1),
      DXGI_FORMAT_R8_UNORM);
  _roughnessTexture = _renderer->CreateTexture(
      Image::fromFile("assets/textures/cerberus_R.png", 1),
      DXGI_FORMAT_R8_UNORM);

  _specularTexture = _renderer->CreateTextureCube("assets/textures/BakerSpecularIBL.dds",
      DXGI_FORMAT_R32G32B32A32_FLOAT, 10);
  _irradianceTexture = _renderer->CreateTextureCube("assets/textures/BakerDiffuseIrradiance.dds",
      DXGI_FORMAT_R32G32B32A32_FLOAT, 1);
  _specularBRDF_LUT = _renderer->CreateTexture(
      Image::fromFile("assets/textures/BakerSpecularBRDF_LUT.dds"),
      DXGI_FORMAT_R32G32B32A32_FLOAT, 1);
}

void DemoApp::InitSamplers() {
  _defaultSampler = _renderer->CreateSamplerState(D3D11_FILTER_ANISOTROPIC,
                                                  D3D11_TEXTURE_ADDRESS_WRAP);
  _computeSampler = _renderer->CreateSamplerState(
      D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP);
}

void DemoApp::InitMeshes() {
  _pbrModel =
      _renderer->CreateMeshBuffer(Mesh::fromFile("assets/model/cerberus.fbx"));
  _skybox =
      _renderer->CreateMeshBuffer(Mesh::fromFile("assets/model/skybox.obj"));
}

void DemoApp::InitLights() {
  // Update shading constant buffer (for pixel shader).
  _shadingConstants.eyePosition = g_camPos;
  _shadingConstants.useIBL = _useIBL;
  _shadingConstants.gamma = 1.f;

  Light light1, light2, light3;
  light1.direction = {-1.0f, 0.0f, 0.0f, 0.f};
  light2.direction = {1.0f, 0.0f, 0.0f, 0.f};
  light3.direction = {0.0f, -1.0f, 0.0f, 0.f};

  light1.radiance = {1.0f, 1.0f, 1.0f, 1.f};
  light2.radiance = {1.0f, 1.0f, 1.0f, 1.f};
  light3.radiance = {1.0f, 1.0f, 1.0f, 1.f};

  light1.enabled = false;
  light2.enabled = false;
  light3.enabled = false;

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
