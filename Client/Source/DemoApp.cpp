//#include <imgui.h>
//#include <imgui_impl_dx11.h>
//#include <imgui_impl_win32.h>

#include "D3DEngine/Renderer/D3D11Renderer.h"
#include "D3DEngine/WinApp/WinApp.h"
#include "DemoApp.h"

#include "D3DEngine/Core/Camera.h"
#include "D3DEngine/ResourceManager/ResourceManager.h"

#define USE_INPUT

#ifdef USE_INPUT
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
bool _enableCamera{false};
IDirectInputDevice8* DIKeyboard;
IDirectInputDevice8* DIMouse;
DIMOUSESTATE mouseLastState{};
LPDIRECTINPUT8 DirectInput{};
#endif

Handle g_sponzaHandle;
std::vector<Handle> g_meshBufHandles;

constexpr size_t kNumLights{3};
std::vector<std::pair<Handle, DX::LightData>> g_lights;

DemoApp* loadedApp{nullptr};

Camera* camera;

void DemoApp::Initialize() {
  if (loadedApp) abort();

  ///////////////////////// DO NOT MODIFY /////////////////////////
  // 윈도우 생성
  WinApp::App_Init();
  // TODO: 윈도우 타이틀이랑 스타일 추가할 것.
  WindowStyleFlags styleFlags = WS_OVERLAPPED;
  hwnd =
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

#ifdef USE_INPUT
	FAILED(DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION,
                           IID_IDirectInput8, (void**)&DirectInput, NULL));

  FAILED(DirectInput->CreateDevice(GUID_SysKeyboard, &DIKeyboard, NULL));
  FAILED(DirectInput->CreateDevice(GUID_SysMouse, &DIMouse, NULL));

  FAILED(DIKeyboard->SetDataFormat(&c_dfDIKeyboard));
  FAILED(DIKeyboard->SetCooperativeLevel(
      hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE));

  FAILED(DIMouse->SetDataFormat(&c_dfDIMouse));
  FAILED(DIMouse->SetCooperativeLevel(
      hwnd, DISCL_NONEXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND));
#endif

	camera = new Camera(SCREEN_WIDTH, SCREEN_HEIGHT);

	Handle geometryVS_Handle =
      DX::LoadShader("Geometry_VS.hlsl", DX::ShaderType::kVertex);
  Handle geometryPS_Handle =
      DX::LoadShader("Geometry_PS.hlsl", DX::ShaderType::kPixel);

	Handle lightVS_Handle =
      DX::LoadShader("Light_VS.hlsl", DX::ShaderType::kVertex);
  Handle lightPS_Handle =
      DX::LoadShader("Light_PS.hlsl", DX::ShaderType::kPixel);

	g_sponzaHandle = DX::LoadModel("Models\\Sponza\\Sponza.gltf");

	DX::LoadTexture("Textures/BakerEnv.dds", DX::TextureType::kAlbedo);
	DX::LoadTexture("Textures/BakerSpecularBRDF_LUT.dds", DX::TextureType::kAlbedo);
	DX::LoadTexture("Textures/BakerDiffuseIrradiance.dds", DX::TextureType::kAlbedo);
	DX::LoadTexture("Textures/BakerSpecularIBL.dds", DX::TextureType::kAlbedo);

	DX::LoadModel("Models\\Sphere\\Sphere.obj");

  _renderer = new DX::D3D11Renderer();
  _renderer->Initialize(hwnd, SCREEN_WIDTH, SCREEN_HEIGHT);

	const DX::ModelData& model = DX::AccessModelData(g_sponzaHandle);

	for (auto mesh : model.meshes) {
    g_meshBufHandles.push_back(_renderer->CreateMesh(mesh));
	}

	// Lights;
  for (int i = 0; i < kNumLights; ++i) {
		Handle lightHandle = _renderer->CreateLight((uint32_t)DX::LightType::kPoint);
    DX::LightData data{.components = {-500.f + i * 500.f, 100, 0, 1},
                       .radiance = {1, 1, 1, 2.f},
                       .constantAttenuation = 1.f,
                       .nearPlane = 1.f,
											 .farPlane = 1000.f,
                       .type = DX::LightType::kPoint};
    g_lights.push_back({lightHandle, data});
	}

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

#ifdef USE_INPUT
    while (ShowCursor(true) < 0);
    ClipCursor(nullptr);
    mouseLastState = {};
#endif

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

#ifdef USE_INPUT
  DIMOUSESTATE mouseCurrState;
  BYTE keyboardState[256];

  DIMouse->Acquire();
  DIKeyboard->Acquire();

  DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);
  DIKeyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);

  if (keyboardState[DIK_ESCAPE] & 0x80) PostMessage(hwnd, WM_DESTROY, 0, 0);

  if (mouseCurrState.rgbButtons[1] & 0x80) {
    while (ShowCursor(false) >= 0);

    if (keyboardState[DIK_Q] & 0x80) {
      camera->MoveDownUp(-dt);
    }
    if (keyboardState[DIK_E] & 0x80) {
      camera->MoveDownUp(dt);
    }
    if (keyboardState[DIK_A] & 0x80) {
      camera->MoveLeftRight(-dt);
    }
    if (keyboardState[DIK_D] & 0x80) {
      camera->MoveLeftRight(dt);
    }
    if (keyboardState[DIK_W] & 0x80) {
      camera->MoveBackForward(dt);
    }
    if (keyboardState[DIK_S] & 0x80) {
      camera->MoveBackForward(-dt);
    }
    if ((mouseCurrState.lX != mouseLastState.lX) ||
        (mouseCurrState.lY != mouseLastState.lY)) {
      camera->RotateAroundXAxis(mouseCurrState.lY);
      camera->RotateAroundYAxis(mouseCurrState.lX);
      mouseLastState = mouseCurrState;
    }

    RECT rect;
    GetClientRect(hwnd, &rect);
    MapWindowPoints(hwnd, nullptr, (POINT*)(&rect), 2);
    ClipCursor(&rect);
  } else {
    while (ShowCursor(true) < 0);
    ClipCursor(nullptr);
    mouseLastState = {};
  }
#endif
}

void DemoApp::Update(float dt) {}

void DemoApp::Render() {
  _renderer->BeginFrame(camera->GetPosition(), camera->GetViewTransform(),
                        camera->GetProjectionTransform());
	

	_renderer->BeginDraw();

	for (auto meshBufHandle : g_meshBufHandles) {
    _renderer->ScheduleMesh(meshBufHandle, XMMatrixIdentity());
	}

	for (auto& [handle, light] : g_lights) {
    _renderer->ScheduleLight(handle, &light);
	}

	_renderer->DrawOpaqueMeshes();
  _renderer->DrawShadows();
  _renderer->DrawLights();

	_renderer->DrawImGui();

	_renderer->EndDraw();
	_renderer->EndFrame();
}
