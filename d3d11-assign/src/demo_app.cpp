#include "demo_app.h"

#include "D3DEngine/WinApp/WinApp.h"

#include "d3d_renderer.h"

#include "model.h"

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "dxguid.lib")

#define USE_FLIPMODE 1	// In order to not show warnings, use flip mode
#define VSYNC_ENABLED 0 // diable v-sync when 0, otherwise v-sync is on

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
	
	XMMATRIX viewProj = _view * _proj;
	model->Draw(viewProj);

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
	XMVECTOR eye = g_camPos;
	XMVECTOR viewDir{ 0.f, 0.f, 1.f };
	XMVECTOR upDir{ 0.f, 1.f, 0.f };
	_view = XMMatrixLookToLH(eye, viewDir, upDir);
	// _view = XMMatrixLookAtLH(eye, viewDir, upDir);
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

	/*Vector4 sunDir{ -1.f, -4.f, -7.f, 0.f };
	sunDir.Normalize();
	Light sunLight{
		.direction = sunDir,
		.color = {.9f, .9f, .9f, 1.f},
		.lightType = LightType::Directional,
		.enabled = true,
	};
	g_lightProperties.PushBackLight(&sunLight);*/


	//Vector4 pointPosition{ -3.f, 5.f, 3.f, 0.f };
	Vector4 pointPosition{ 0.f, 0.f, -5.f, 0.f };
	pointPosition.Normalize();
	Light pointLight{
		.position = pointPosition,
		.color = { 1.f, 1.f, 1.f, 1.f },
		.constAtt = 1.f,
		.lightType = LightType::Point,
		.enabled = true
	};
	g_lightProperties.PushBackLight(&pointLight);


}
