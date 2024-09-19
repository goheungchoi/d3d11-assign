#include "demo_app.h"

#include "D3DEngine/WinApp/WinApp.h"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include "d3d_renderer.h"

#include "model.h"

#include "camera.h"

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "dxguid.lib")

#define USE_FLIPMODE 1	// In order to not show warnings, use flip mode
#define VSYNC_ENABLED 0 // diable v-sync when 0, otherwise v-sync is on
#define USE_GUI 0
#define USE_CAM 1

DemoApp* loadedApp{ nullptr };

void DemoApp::Initialize() {
	if (loadedApp) abort();

	///////////////////////// DO NOT MODIFY /////////////////////////
	// 윈도우 생성
	WinApp::App_Init();
	// TODO: 윈도우 타이틀이랑 스타일 추가할 것.
	WindowStyleFlags styleFlags = WS_OVERLAPPED;
	hWindow = WinApp::App_CreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, TITLE, styleFlags);

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
	if (FAILED(_renderer->Initialize(hWindow)))
		throw std::runtime_error("Initialization of D3D 11 failed!");

	InitTransformMatrices();
	InitCamera();
	InitModels();
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


}

void DemoApp::Update(float dt) {
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
	
#if USE_CAM == 1
	_view = _camera->GetViewTransform();
#endif
	XMMATRIX viewProj = _view * _proj;
	model->Draw(viewProj);

#if USE_GUI == 1
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (ImGui::Begin("Point Light")) {
		ImGui::Text("Light Position: ");

		ImGui::SliderFloat("x", &g_lightProperties.lights[1].position.x, -10, 10);
		ImGui::SliderFloat("y", &g_lightProperties.lights[1].position.y, -10, 10);
		ImGui::SliderFloat("z", &g_lightProperties.lights[1].position.z, -10, 10);
	} ImGui::End();

	// Rendering
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
#endif

	_renderer->EndDraw();
#ifndef NDEBUG 
	++count;
#endif
}

void DemoApp::InitTransformMatrices()
{
	// Setup the vertical field of view
	float vfov = PI_F / 2.f;	// 90 degree field of view
	// Get the screen aspect ratio
	float aspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;

	// Create the projection matrix
	_proj = XMMatrixPerspectiveFovLH(vfov, aspectRatio, 0.01f, 100.f);
	// Create the Orthographic projection matrix
	//_proj = XMMatrixOrthographicLH((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.01f, 100.f);
}

void DemoApp::InitCamera()
{
#if USE_CAM == 1
	_camera = new Camera(GetModuleHandle(NULL), hWindow);
	_view = _camera->GetViewTransform();
#else
	XMVECTOR eye = g_camPos;
	XMVECTOR viewDir{ 0.f, 0.f, 1.f };
	XMVECTOR upDir{ 0.f, 1.f, 0.f };
	_view = XMMatrixLookToLH(eye, viewDir, upDir);
#endif
}

void DemoApp::InitModels()
{
	model = new Model(
		_renderer->_device,
		_renderer->_deviceContext,
		"assets/backpack/backpack.obj"
	);
}

void DemoApp::InitLights()
{
	g_lightProperties = {};

	SetGlobalEyePosition(g_camPos);
	SetGlobalAmbient({ 0.01f, 0.01f, 0.01f, 1.f });

	Vector4 sunDir{ -1.f, -4.f, -7.f, 0.f };
	sunDir.Normalize();
	Light sunLight{
		.direction = sunDir,
		.color = {.9f, .9f, .9f, 1.f},
		.lightType = LightType::Directional,
		.enabled = true,
	};
	PushBackLight(&sunLight);


	//Vector4 pointPosition{ -3.f, 5.f, 3.f, 0.f };
	Vector4 pointPosition{ 0.f, -5.f, -5.f, 0.f };
	pointPosition.Normalize();
	Light pointLight{
		.position = pointPosition,
		.color = { 1.f, 1.f, 1.f, 1.f },
		.constAtt = 1.f,
		.linearAtt = 1.f,
		.lightType = LightType::Point,
		.enabled = true
	};
	PushBackLight(&pointLight);

}

void DemoApp::InitImgui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hWindow);
	ImGui_ImplDX11_Init(_renderer->_device, _renderer->_deviceContext);
}
