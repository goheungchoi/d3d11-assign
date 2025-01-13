#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include "D3DEngine/Renderer/D3D11Renderer.h"
#include "D3DEngine/WinApp/WinApp.h"
#include "DemoApp.h"

#include "D3DEngine/ResourceManager/ResourceManager.h"

DemoApp* loadedApp{nullptr};

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

	Handle geometryVS_Handle =
      DX::LoadShader("Geometry_VS.hlsl", DX::ShaderType::kVertex);
  Handle geometryPS_Handle =
      DX::LoadShader("Geometry_PS.hlsl", DX::ShaderType::kPixel);

	Handle lightVS_Handle =
      DX::LoadShader("Light_VS.hlsl", DX::ShaderType::kVertex);
  Handle lightPS_Handle =
      DX::LoadShader("Light_PS.hlsl", DX::ShaderType::kPixel);

  _renderer = new DX::D3D11Renderer();
  _renderer->Initialize(hwnd, SCREEN_WIDTH, SCREEN_HEIGHT);

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

void DemoApp::Update(float dt) {}

void DemoApp::Render() {
  _renderer->BeginFrame(XMMatrixIdentity(), XMMatrixIdentity());
	



	_renderer->DrawImGui();
	_renderer->EndFrame();
}
