#include "demo_app.h"

#include "D3DEngine/WinApp/WinApp.h"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include <imgui_impl_dx11.h>

#include "d3d_renderer.h"

// #include "model.h"
#include "scene/cube.h"
#include "scene/system.h"

#include "camera.h"

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "dxguid.lib")

#define USE_FLIPMODE 1	// In order to not show warnings, use flip mode
#define VSYNC_ENABLED 0 // diable v-sync when 0, otherwise v-sync is on
#define USE_GUI 1
#define USE_CAM 0

std::vector<Cube*> cubes;
std::vector<XMVECTOR> cubePositions;

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

	// TODO: _view update
	XMVECTOR eye = g_camPos;
	XMVECTOR viewDir{ 0.f, 0.f, 1.f };
	XMVECTOR upDir{ 0.f, 1.f, 0.f };
	_view = XMMatrixLookAtLH(g_camPos, { 0.f, 0.f, 0.f, 0.f }, upDir);

	_proj = XMMatrixPerspectiveFovLH(vfov, aspectRatio, nearZ, farZ);
	g_cbPerFrame.viewProj = _view * _proj;

	// Update models
	cubes[0]->RotateY(dt * 10.f);
	cubes[1]->RotateY(-dt * 150.f);
	cubes[2]->RotateY(dt * 100.f);

	cubes[0]->SetTranslation(cubePositions[0]);
	cubes[1]->SetTranslation(cubePositions[1]);
	cubes[2]->SetTranslation(cubePositions[2]);

	for (auto* cube : cubes) {
		cube->UpdateLocalTransform();
	}

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
	/*XMMATRIX viewProj = _view * _proj;
	model->Draw(viewProj);*/

	for (auto* cube : cubes) {
		cube->Draw();
	}

#if USE_GUI == 1
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (ImGui::Begin("Mesh0")) {
		ImGui::Text("Position: ");

		ImGui::SliderFloat("x", &cubePositions[0].m128_f32[0], -10, 10);
		ImGui::SliderFloat("y", &cubePositions[0].m128_f32[1], -10, 10);
		ImGui::SliderFloat("z", &cubePositions[0].m128_f32[2], -10, 10);
	} ImGui::End();

	if (ImGui::Begin("Mesh1")) {
		ImGui::Text("Position: ");

		ImGui::SliderFloat("x", &cubePositions[1].m128_f32[0], -10, 10);
		ImGui::SliderFloat("y", &cubePositions[1].m128_f32[1], -10, 10);
		ImGui::SliderFloat("z", &cubePositions[1].m128_f32[2], -10, 10);
	} ImGui::End();

	if (ImGui::Begin("Mesh2")) {
		ImGui::Text("Position: ");

		ImGui::SliderFloat("x", &cubePositions[2].m128_f32[0], -10, 10);
		ImGui::SliderFloat("y", &cubePositions[2].m128_f32[1], -10, 10);
		ImGui::SliderFloat("z", &cubePositions[2].m128_f32[2], -10, 10);
	} ImGui::End();

	if (ImGui::Begin("Camera")) {
		ImGui::Text("Position: ");

		ImGui::SliderFloat("x", &g_camPos.x, -10, 10);
		ImGui::SliderFloat("y", &g_camPos.y, -10, 10);
		ImGui::SliderFloat("z", &g_camPos.z, -10, 10);
	
		ImGui::Text("Field of view: ");
		ImGui::SliderFloat("fov", &vfov, PI_F / 4.f, PI_F / 2.f);

		ImGui::Text("Z Plane: ");
		ImGui::SliderFloat("near", &nearZ, 0.001, 100);
		ImGui::SliderFloat("far", &farZ, 100, 1000);

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
	vfov = PI_F / 2.f;	// 90 degree field of view
	// Get the screen aspect ratio
	aspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;

	nearZ = 0.01;
	farZ = 100.f;

	// Create the projection matrix
	_proj = XMMatrixPerspectiveFovLH(vfov, aspectRatio, nearZ, farZ);
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
	//_view = XMMatrixLookToLH(eye, viewDir, upDir);

	_view = XMMatrixLookAtLH(g_camPos, { 0.f, 0.f, 0.f, 0.f }, upDir);
#endif
}

void DemoApp::InitModels()
{
	/*model = new Model(
		_renderer->_device,
		_renderer->_deviceContext,
		"assets/backpack/backpack.obj"
	);*/

	cubePositions.push_back({ 0.f, 0.f, 0.f });
	cubePositions.push_back({ 4.f, 0.f, 0.f });
	cubePositions.push_back({ 4.f, 0.f, 0.f });

	Cube* cube1 = new Cube(_renderer->_device, _renderer->_deviceContext);
	
	Cube* cube2 = new Cube(_renderer->_device, _renderer->_deviceContext);
	cube1->AddChildSceneComponent(cube2);
	cube2->SetScale({ .2f, .2f, .2f });

	Cube* cube3 = new Cube(_renderer->_device, _renderer->_deviceContext);
	cube2->AddChildSceneComponent(cube3);
	cube3->SetScale({ 0.5f, 0.5f, 0.5f });

	cubes.push_back(cube1);
	cubes.push_back(cube2);
	cubes.push_back(cube3);
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
