#include "demo_app.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include "D3DEngine/Renderer/D3D11Renderer.h"
#include "D3DEngine/WinApp/WinApp.h"
#include "camera.h"
#include <dinput.h>

#include "animation/animator.h"
#include "animation/model.h"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

#define USE_FLIPMODE 1   // In order to not show warnings, use flip mode
#define VSYNC_ENABLED 0  // diable v-sync when 0, otherwise v-sync is on
#define USE_GUI 1

DemoApp* loadedApp{nullptr};

XMVECTOR g_eyePos{};

const float g_sunDist{10000.f};
XMVECTOR g_sunPos{0.f, 1000.f, -10.f, 1.f};
XMVECTOR g_sunDir{0.01f, -1.f, 0.01f, 1.f};

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
  InitShadowPass();
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
		
		while (ShowCursor(true) < 0);
    ClipCursor(nullptr);
    mouseLastState = {};

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

  if (mouseCurrState.rgbButtons[1] & 0x80) {
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

  //g_camPos.m128_f32[2] = g_camDist;
  //XMVECTOR eye = g_camPos;
  //XMVECTOR viewDir{0.f, 0.f, -1.f};
  //XMVECTOR upDir{0.f, 1.f, 0.f};
  //// _view = XMMatrixLookToLH(eye, viewDir, upDir);
  //XMMATRIX transform = XMMatrixTranslationFromVector(g_camPos) *
  //                     XMMatrixRotationY(XMConvertToRadians(rotation));
  //XMVECTOR dump1, dump2;
  //XMMatrixDecompose(&dump1, &dump2, &eye, transform);
  //g_eyePos = eye;
  //_view = XMMatrixLookAtLH(eye, {0, 0, 0, 0}, upDir);

	_animator->UpdateAnimation(dt);
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

	XMMATRIX groundTransform = 
      XMMatrixScaling(10.f, 1.f, 10.f) *
			XMMatrixTranslation(0.f, -100.f, 0.f);

  _renderer->BeginDraw();

  _renderer->_context->IASetPrimitiveTopology(
      D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  _renderer->_context->RSSetState(_defaultRasterizerState.Get());

  _view = _camera->GetViewTransform();

	// Shadow transformation
  g_sunDir = XMVector4Normalize(g_sunDir);
  XMVECTOR sunFocus = XMVectorAdd(g_sunPos, 
		XMVectorMultiply(g_sunDir, XMVECTOR{g_sunDist, g_sunDist, g_sunDist, 1.f}));
  _shadingConstants.lights[0].direction = g_sunDir;

  _shadowView = XMMatrixLookAtLH(g_sunPos, sunFocus, {0, 1, 0, 0});
  _shadowProj = XMMatrixOrthographicLH(2048, 2048, 0.01f, 10000.f);

  _transformConstants.view = XMMatrixTranspose(_view);
  _transformConstants.proj = XMMatrixTranspose(_proj);
  _transformConstants.sceneRotation =
      XMMatrixTranspose(XMMatrixScaling(0.8, 0.8, 0.8));
  _transformConstants.shadowViewProj =
      XMMatrixTranspose(XMMatrixMultiply(_shadowView, _shadowProj));
  _renderer->CopyDataToDeviceBuffer(_cboTransform, &_transformConstants);

  _shadingConstants.eyePosition = _camera->GetPosition();

  _renderer->CopyDataToDeviceBuffer(_cboShading, &_shadingConstants);

  _renderer->_context->VSSetConstantBuffers(0, 1, _cboTransform.GetAddressOf());
  _renderer->_context->PSSetConstantBuffers(0, 1, _cboShading.GetAddressOf());

	/////////////////////// Draw depth buffer /////////////////////////////
	// Change the render target to the depth frame buffer
  _renderer->_context->OMSetRenderTargets(1, _depthBuffer.rtv.GetAddressOf(),
                                          _depthBuffer.dsv.Get());
  _renderer->_context->VSSetShader(_shadowProgram.vertexShader.Get(), nullptr,
                                   0);
  _renderer->_context->PSSetShader(_shadowProgram.pixelShader.Get(), nullptr,
                                   0);
  _renderer->_context->OMSetDepthStencilState(_renderer->_depthStencilState,
                                              0);
  {
    D3D11_VIEWPORT viewport{.TopLeftX = 0,
                            .TopLeftY = 0,
                            .Width = 2048,
                            .Height = 2048,
                            .MinDepth = 0,
                            .MaxDepth = 1};
    _renderer->_context->RSSetViewports(1, &viewport);
  }
	float clear_color[4] = {0, 0, 0, 0};
  _renderer->_context->ClearRenderTargetView(_depthBuffer.rtv.Get(),
                                             clear_color);
  _renderer->_context->ClearDepthStencilView(_depthBuffer.dsv.Get(),
                                             D3D11_CLEAR_DEPTH, 1.0f, 0);

	// PBR
  _renderer->_context->IASetInputLayout(_shadowProgram.inputLayout.Get());
  _renderer->_context->IASetVertexBuffers(0, 1,
                                          _pbrModel.vertexBuffer.GetAddressOf(),
                                          &_pbrModel.stride, &_pbrModel.offset);
  _renderer->_context->IASetIndexBuffer(_pbrModel.indexBuffer.Get(),
                                        DXGI_FORMAT_R32_UINT, 0);
  _renderer->_context->DrawIndexed(_pbrModel.numElements, 0, 0);

	// Ground
	_transformConstants.sceneRotation = XMMatrixTranspose(groundTransform);
  _renderer->CopyDataToDeviceBuffer(_cboTransform, &_transformConstants);

	_renderer->_context->IASetVertexBuffers(0, 1,
                                          _ground.vertexBuffer.GetAddressOf(),
                                          &_ground.stride, &_ground.offset);
  _renderer->_context->IASetIndexBuffer(_ground.indexBuffer.Get(),
                                        DXGI_FORMAT_R32_UINT, 0);
  _renderer->_context->DrawIndexed(_ground.numElements, 0, 0);

	///////	Animation Model	////////
	
	{
    XMMATRIX topMat = _view * _proj;
    const auto& boneTransforms = _animator->GetFinalBoneTransforms();
    for (ModelMesh& mesh : _model->_meshes) {
      mesh._cbPerFrame.viewProj = _transformConstants.shadowViewProj;
      mesh._cbPerObject.model = XMMatrixTranspose(mesh._modelTransform);
      mesh._cbPerObject.inverseTransposeModel =
          XMMatrixInverse(nullptr, mesh._modelTransform);
      memcpy(mesh._cbPerObject.boneTransforms, boneTransforms.data(),
             sizeof(XMMATRIX) * MAX_BONES);

			/* INPUT ASSEMBLER STAGE */
      mesh._context->IASetInputLayout(_skeletalShadowLayout.Get());
      mesh._context->IASetVertexBuffers(0, 1, &mesh._vbo, &mesh._vbStride,
                                        &mesh._vbOffset);
      mesh._context->IASetIndexBuffer(mesh._ibo, DXGI_FORMAT_R32_UINT,
                                      mesh._ibOffset);

      /* VERTEX STAGE */
      mesh._context->VSSetShader(_skeletalShadowVS.Get(), nullptr, 0);
      D3D11_MAPPED_SUBRESOURCE cbPerFrameSubresource;
      mesh._context->Map(mesh._cboPerFrame, NULL, D3D11_MAP_WRITE_DISCARD, NULL,
                         &cbPerFrameSubresource);
      memcpy(cbPerFrameSubresource.pData, &mesh._cbPerFrame,
             sizeof(cbPerFrame));
      mesh._context->Unmap(mesh._cboPerFrame, NULL);
      mesh._context->VSSetConstantBuffers(0, 1, &mesh._cboPerFrame);
      D3D11_MAPPED_SUBRESOURCE cbPerObjectSubresource;
      mesh._context->Map(mesh._cboPerObject, NULL, D3D11_MAP_WRITE_DISCARD,
                         NULL, &cbPerObjectSubresource);
      memcpy(cbPerObjectSubresource.pData, &mesh._cbPerObject,
             sizeof(cbPerObject));
      mesh._context->Unmap(mesh._cboPerObject, NULL);
      mesh._context->VSSetConstantBuffers(1, 1, &mesh._cboPerObject);

      mesh._context->DrawIndexed(mesh._indexCount, 0, 0);
		}
	}

	/////////////////////// End depth buffer //////////////////////////////

  // Draw skybox.
  {
    D3D11_VIEWPORT viewport{.TopLeftX = 0,
                            .TopLeftY = 0,
                            .Width = SCREEN_WIDTH,
                            .Height = SCREEN_HEIGHT,
                            .MinDepth = 0,
                            .MaxDepth = 1};
    _renderer->_context->RSSetViewports(1, &viewport);
  }

	_renderer->_context->VSSetConstantBuffers(0, 1, _cboTransform.GetAddressOf());
  _renderer->_context->PSSetConstantBuffers(0, 1, _cboShading.GetAddressOf());

  _transformConstants.view = XMMatrixTranspose(_view);
  _transformConstants.proj = XMMatrixTranspose(_proj);
  _transformConstants.sceneRotation =
      XMMatrixTranspose(XMMatrixScaling(0.8, 0.8, 0.8));
  _transformConstants.shadowViewProj =
      XMMatrixTranspose(_shadowView * _shadowProj);
  _renderer->CopyDataToDeviceBuffer(_cboTransform, &_transformConstants);

  _renderer->_context->OMSetRenderTargets(1, &_renderer->_backbufferRTV,
                                          _renderer->_depthStencilView);

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
    _renderer->_context->OMSetDepthStencilState(_renderer->_depthStencilState,
                                                0);
    _renderer->_context->DrawIndexed(_skybox.numElements, 0, 0);
  }

  // Draw PBR model.
  ID3D11ShaderResourceView* const pbrModelSRVs[] = {
    _albedoTexture.srv.Get(),
    _normalTexture.srv.Get(),
    _metalnessTexture.srv.Get(),
    _roughnessTexture.srv.Get(),
    _specularTexture.srv.Get(),
    _irradianceTexture.srv.Get(),
    _specularBRDF_LUT.srv.Get(),
    _depthBuffer.depthSRV.Get(),
  };
  ID3D11SamplerState* const pbrModelSamplers[] = {
      _defaultSampler.Get(),
      _spBRDF_Sampler.Get(),
			_shadowSampler.Get()
  };


  _renderer->_context->IASetInputLayout(_pbrProgram.inputLayout.Get());
  _renderer->_context->IASetVertexBuffers(0, 1,
                                          _pbrModel.vertexBuffer.GetAddressOf(),
                                          &_pbrModel.stride, &_pbrModel.offset);
  _renderer->_context->IASetIndexBuffer(_pbrModel.indexBuffer.Get(),
                                        DXGI_FORMAT_R32_UINT, 0);
  _renderer->_context->VSSetShader(_pbrProgram.vertexShader.Get(), nullptr, 0);
  _renderer->_context->PSSetShader(_pbrProgram.pixelShader.Get(), nullptr, 0);
  _renderer->_context->PSSetShaderResources(0, 8, pbrModelSRVs);
  _renderer->_context->PSSetSamplers(0, 3, pbrModelSamplers);
  _renderer->_context->OMSetDepthStencilState(_renderer->_depthStencilState,
                                              0);
  _renderer->_context->DrawIndexed(_pbrModel.numElements, 0, 0);

	// Draw Ground Model
  ID3D11ShaderResourceView* const groundModelSRVs[] = {
      _groundATexture.srv.Get(),   _groundNTexture.srv.Get(),
      _groundMTexture.srv.Get(),   _groundRTexture.srv.Get(),
      _specularTexture.srv.Get(),  _irradianceTexture.srv.Get(),
      _specularBRDF_LUT.srv.Get(), _depthBuffer.depthSRV.Get(),
  };

  _transformConstants.sceneRotation = XMMatrixTranspose(groundTransform);
  _renderer->CopyDataToDeviceBuffer(_cboTransform, &_transformConstants);
	_renderer->_context->IASetVertexBuffers(0, 1,
                                          _ground.vertexBuffer.GetAddressOf(),
                                          &_ground.stride, &_ground.offset);
  _renderer->_context->IASetIndexBuffer(_ground.indexBuffer.Get(),
                                        DXGI_FORMAT_R32_UINT, 0);
  _renderer->_context->VSSetShader(_groundProgram.vertexShader.Get(), nullptr,
                                   0);
  _renderer->_context->PSSetShader(_groundProgram.pixelShader.Get(), nullptr,
                                   0);
	_renderer->_context->PSSetShaderResources(0, 8, groundModelSRVs);
	_renderer->_context->DrawIndexed(_ground.numElements, 0, 0);


	///////	Draw Animation Model	////////////////////////////////////////

	{
    XMMATRIX topMat = _view * _proj;
    const auto& boneTransforms = _animator->GetFinalBoneTransforms();
    for (ModelMesh& mesh : _model->_meshes) {
      mesh._cbPerFrame.viewProj = XMMatrixTranspose(topMat);
      mesh._cbPerObject.model = XMMatrixTranspose(mesh._modelTransform);
      mesh._cbPerObject.inverseTransposeModel =
          XMMatrixInverse(nullptr, mesh._modelTransform);
      memcpy(mesh._cbPerObject.boneTransforms, boneTransforms.data(),
             sizeof(XMMATRIX) * MAX_BONES);

      /* INPUT ASSEMBLER STAGE */
      mesh._context->IASetVertexBuffers(0, 1, &mesh._vbo, &mesh._vbStride,
                                        &mesh._vbOffset);
      mesh._context->IASetIndexBuffer(mesh._ibo, DXGI_FORMAT_R32_UINT,
                                      mesh._ibOffset);
      mesh._context->IASetInputLayout(mesh._inputLayout);

      /* VERTEX STAGE */
      mesh._context->VSSetShader(mesh._vs, nullptr, 0);
      D3D11_MAPPED_SUBRESOURCE cbPerFrameSubresource;
      mesh._context->Map(mesh._cboPerFrame, NULL, D3D11_MAP_WRITE_DISCARD, NULL,
                         &cbPerFrameSubresource);
      memcpy(cbPerFrameSubresource.pData, &mesh._cbPerFrame,
             sizeof(cbPerFrame));
      mesh._context->Unmap(mesh._cboPerFrame, NULL);
      mesh._context->VSSetConstantBuffers(0, 1, &mesh._cboPerFrame);
      D3D11_MAPPED_SUBRESOURCE cbPerObjectSubresource;
      mesh._context->Map(mesh._cboPerObject, NULL, D3D11_MAP_WRITE_DISCARD,
                         NULL, &cbPerObjectSubresource);
      memcpy(cbPerObjectSubresource.pData, &mesh._cbPerObject,
             sizeof(cbPerObject));
      mesh._context->Unmap(mesh._cboPerObject, NULL);
      mesh._context->VSSetConstantBuffers(1, 1, &mesh._cboPerObject);

      /* PIXEL STAGE */
      mesh._context->PSSetShader(mesh._ps, nullptr, 0);
      // Diffuse
      //_context->PSSetShaderResources(0, 1, &textures[0].textureView);
      //_context->PSSetSamplers(0, 1, &textures[0].samplerState);
      //// Specular
      //_context->PSSetShaderResources(1, 1, &textures[1].textureView);
      //_context->PSSetSamplers(1, 1, &textures[1].samplerState);
      //// Normal
      //_context->PSSetShaderResources(2, 1, &textures[2].textureView);
      //_context->PSSetSamplers(2, 1, &textures[2].samplerState);
      // TODO: Shadow maps
      ID3D11SamplerState* const samplers[] = {
          _defaultSampler.Get(), _defaultSampler.Get(), _defaultSampler.Get()};
      mesh._context->PSSetSamplers(0, 3, samplers);

      // Bind constant buffers
      // Material properties
      D3D11_MAPPED_SUBRESOURCE cbMaterialPropertiesSubresource;
      mesh._context->Map(mesh._cboMaterialProperties, NULL,
                         D3D11_MAP_WRITE_DISCARD, NULL,
                         &cbMaterialPropertiesSubresource);
      memcpy(cbMaterialPropertiesSubresource.pData, &mesh._cbMaterialProperties,
             sizeof(cbMaterialProperties));
      mesh._context->Unmap(mesh._cboMaterialProperties, NULL);
      mesh._context->PSSetConstantBuffers(0, 1, &mesh._cboMaterialProperties);
      // Light properties
      D3D11_MAPPED_SUBRESOURCE cbLightPropertiesSubresource;
      mesh._context->Map(mesh._cboLightProperties, NULL,
                         D3D11_MAP_WRITE_DISCARD, NULL,
                         &cbLightPropertiesSubresource);
      memcpy(cbLightPropertiesSubresource.pData, &_shadingConstants, sizeof(cbShadingConstants));
      mesh._context->Unmap(mesh._cboLightProperties, NULL);
      mesh._context->PSSetConstantBuffers(1, 1, &mesh._cboLightProperties);

      // Start sending commands to the gpu.
      mesh._context->DrawIndexed(mesh._indexCount, 0, 0);
    }
  }

	//////////////////////////////////////////////////////////////////////
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
    ImGui::Text("Use PCF: ");
    ImGui::Checkbox("UsePCF", (bool*)&_shadingConstants.usePCF);
    ImGui::Text("Light Properties: ");
    ImGui::Checkbox("Enable Light", (bool*)&_shadingConstants.lights[0].enabled);
    ImGui::SliderFloat3("Sun Direction", g_sunDir.m128_f32, -1, 1);
		ImGui::SliderFloat3("Light Radiance",
                        (float*) & (_shadingConstants.lights[0].radiance), 0.f, 10.f);
		/*ImGui::Text("Light 2 Properties: ");
    ImGui::Checkbox("Enable Light 2", (bool*)&_shadingConstants.lights[1].enabled);
    ImGui::SliderFloat3("Light 2 Radiance",
                        (float*)&(_shadingConstants.lights[1].radiance), 0.f,
                        10.f);
    ImGui::Text("Light 3 Properties: ");
    ImGui::Checkbox("Enable Light 3", (bool*)&_shadingConstants.lights[2].enabled);
    ImGui::SliderFloat3("Light 3 Radiance",
                        (float*)&(_shadingConstants.lights[2].radiance), 0.f,
                        10.f);*/

		ImTextureID imgID = (ImTextureID)(uintptr_t)_depthBuffer.depthSRV.Get();
    ImGui::Image(imgID, ImVec2(400, 400));
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

void DemoApp::InitShadowPass() {

	// Shadow shader program
  const std::vector<D3D11_INPUT_ELEMENT_DESC> shadowInputLayout = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
      D3D11_INPUT_PER_VERTEX_DATA, 0},
  };
	
	_shadowProgram = _renderer->CreateShaderProgram(
    CompileShaderFromFile(L"shaders/Shadow_VS.hlsl", "main", "vs_5_0"),
    CompileShaderFromFile(L"shaders/Shadow_PS.hlsl", "main", "ps_5_0"),
    &shadowInputLayout
  );

	// Skeletal vertex shader
  std::vector<uint8_t> vsByteData =
      CompileShaderFromFile(L"shaders/SkeletalShadow_VS.hlsl", "main", "vs_5_0");

  // Create the input layout of the shader
  // Input layout descriptor
  D3D11_INPUT_ELEMENT_DESC vsInputLayoutDescriptors[] = {
      // Position layout
      D3D11_INPUT_ELEMENT_DESC{
          .SemanticName = "POSITION",
          .SemanticIndex = 0U,
          .Format = DXGI_FORMAT_R32G32B32_FLOAT,
          .InputSlot = 0,
          .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
          .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
          .InstanceDataStepRate = 0},
      // Texture coordinate
      D3D11_INPUT_ELEMENT_DESC{
          .SemanticName = "TEXCOORD",
          .SemanticIndex = 0U,
          .Format = DXGI_FORMAT_R32G32_FLOAT,
          .InputSlot = 0,
          .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
          .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
          .InstanceDataStepRate = 0},
      // Normal layout
      D3D11_INPUT_ELEMENT_DESC{
          .SemanticName = "NORMAL",
          .SemanticIndex = 0U,
          .Format = DXGI_FORMAT_R32G32B32_FLOAT,
          .InputSlot = 0,
          .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
          .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
          .InstanceDataStepRate = 0},
      // Tangent layout
      D3D11_INPUT_ELEMENT_DESC{
          .SemanticName = "TANGENT",
          .SemanticIndex = 0U,
          .Format = DXGI_FORMAT_R32G32B32_FLOAT,
          .InputSlot = 0,
          .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
          .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
          .InstanceDataStepRate = 0},
      //------------------------------------------------------
      // Bone IDs layout
      D3D11_INPUT_ELEMENT_DESC{
          .SemanticName = "BONE_IDS",
          .SemanticIndex = 0U,
          .Format = DXGI_FORMAT_R32_SINT,
          .InputSlot = 0,
          .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
          .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
          .InstanceDataStepRate = 0},
      D3D11_INPUT_ELEMENT_DESC{
          .SemanticName = "BONE_IDS",
          .SemanticIndex = 1U,
          .Format = DXGI_FORMAT_R32_SINT,
          .InputSlot = 0,
          .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
          .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
          .InstanceDataStepRate = 0},
      D3D11_INPUT_ELEMENT_DESC{
          .SemanticName = "BONE_IDS",
          .SemanticIndex = 2U,
          .Format = DXGI_FORMAT_R32_SINT,
          .InputSlot = 0,
          .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
          .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
          .InstanceDataStepRate = 0},
      D3D11_INPUT_ELEMENT_DESC{
          .SemanticName = "BONE_IDS",
          .SemanticIndex = 3U,
          .Format = DXGI_FORMAT_R32_SINT,
          .InputSlot = 0,
          .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
          .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
          .InstanceDataStepRate = 0},
      D3D11_INPUT_ELEMENT_DESC{
          .SemanticName = "BONE_IDS",
          .SemanticIndex = 4U,
          .Format = DXGI_FORMAT_R32_SINT,
          .InputSlot = 0,
          .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
          .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
          .InstanceDataStepRate = 0},
      D3D11_INPUT_ELEMENT_DESC{
          .SemanticName = "BONE_IDS",
          .SemanticIndex = 5U,
          .Format = DXGI_FORMAT_R32_SINT,
          .InputSlot = 0,
          .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
          .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
          .InstanceDataStepRate = 0},
      D3D11_INPUT_ELEMENT_DESC{
          .SemanticName = "BONE_IDS",
          .SemanticIndex = 6U,
          .Format = DXGI_FORMAT_R32_SINT,
          .InputSlot = 0,
          .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
          .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
          .InstanceDataStepRate = 0},
      D3D11_INPUT_ELEMENT_DESC{
          .SemanticName = "BONE_IDS",
          .SemanticIndex = 7U,
          .Format = DXGI_FORMAT_R32_SINT,
          .InputSlot = 0,
          .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
          .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
          .InstanceDataStepRate = 0},
      // Bone weigths layout
      D3D11_INPUT_ELEMENT_DESC{
          .SemanticName = "BONE_WEIGHTS",
          .SemanticIndex = 0U,
          .Format = DXGI_FORMAT_R32_FLOAT,
          .InputSlot = 0,
          .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
          .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
          .InstanceDataStepRate = 0},
      D3D11_INPUT_ELEMENT_DESC{
          .SemanticName = "BONE_WEIGHTS",
          .SemanticIndex = 1U,
          .Format = DXGI_FORMAT_R32_FLOAT,
          .InputSlot = 0,
          .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
          .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
          .InstanceDataStepRate = 0},
      D3D11_INPUT_ELEMENT_DESC{
          .SemanticName = "BONE_WEIGHTS",
          .SemanticIndex = 2U,
          .Format = DXGI_FORMAT_R32_FLOAT,
          .InputSlot = 0,
          .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
          .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
          .InstanceDataStepRate = 0},
      D3D11_INPUT_ELEMENT_DESC{
          .SemanticName = "BONE_WEIGHTS",
          .SemanticIndex = 3U,
          .Format = DXGI_FORMAT_R32_FLOAT,
          .InputSlot = 0,
          .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
          .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
          .InstanceDataStepRate = 0},
      D3D11_INPUT_ELEMENT_DESC{
          .SemanticName = "BONE_WEIGHTS",
          .SemanticIndex = 4U,
          .Format = DXGI_FORMAT_R32_FLOAT,
          .InputSlot = 0,
          .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
          .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
          .InstanceDataStepRate = 0},
      D3D11_INPUT_ELEMENT_DESC{
          .SemanticName = "BONE_WEIGHTS",
          .SemanticIndex = 5U,
          .Format = DXGI_FORMAT_R32_FLOAT,
          .InputSlot = 0,
          .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
          .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
          .InstanceDataStepRate = 0},
      D3D11_INPUT_ELEMENT_DESC{
          .SemanticName = "BONE_WEIGHTS",
          .SemanticIndex = 6U,
          .Format = DXGI_FORMAT_R32_FLOAT,
          .InputSlot = 0,
          .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
          .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
          .InstanceDataStepRate = 0},
      D3D11_INPUT_ELEMENT_DESC{
          .SemanticName = "BONE_WEIGHTS",
          .SemanticIndex = 7U,
          .Format = DXGI_FORMAT_R32_FLOAT,
          .InputSlot = 0,
          .AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
          .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
          .InstanceDataStepRate = 0},
  };
  
	_renderer->_device->CreateInputLayout(
      vsInputLayoutDescriptors, (UINT)std::size(vsInputLayoutDescriptors),
      vsByteData.data(), vsByteData.size(), _skeletalShadowLayout.GetAddressOf());

	_renderer->_device->CreateVertexShader(vsByteData.data(), vsByteData.size(),
                                         NULL, _skeletalShadowVS.GetAddressOf());

	// Shadow sampler
	D3D11_SAMPLER_DESC samplerDesc{};
  samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
  samplerDesc.MaxAnisotropy = 1;
  samplerDesc.MinLOD = 0;
  samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
  samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
  samplerDesc.BorderColor[0] = 0.f;
  samplerDesc.BorderColor[1] = 0.f;
  samplerDesc.BorderColor[2] = 0.f;
  samplerDesc.BorderColor[3] = 1.f;
	_renderer->_device->CreateSamplerState(&samplerDesc,
                                         _shadowSampler.GetAddressOf());

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

	// Depth stencil state
	_renderer->_device->CreateDepthStencilState(
      &depthStencilDesc, _shadowDepthStencilState.GetAddressOf());

	// Depth buffer
  _depthBuffer = _renderer->CreateFrameBuffer(2048, 2048, 1,
                                              DXGI_FORMAT_R16G16B16A16_FLOAT,
                                   DXGI_FORMAT_D24_UNORM_S8_UINT);
  _shadingConstants.shadowMapSize = 2048;

	// Shadow transformation
  g_sunDir = XMVector4Normalize(g_sunDir);
  XMVECTOR sunFocus = XMVectorAdd(
      g_sunPos, XMVectorMultiply(
                    g_sunDir, XMVECTOR{g_sunDist, g_sunDist, g_sunDist, 1.f}));
  
  _shadowView = XMMatrixLookAtLH(g_sunPos, 
																 sunFocus,
                                 {0, 1, 0, 0});
  _shadowProj = XMMatrixOrthographicLH(2048, 2048, 0.01f, 10000.f);
}

void DemoApp::InitTransformMatrices() {
  // Setup the vertical field of view
  float vfov = PI_F / 2.f;  // 90 degree field of view
  // Get the screen aspect ratio
  float aspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;

  // Create the projection matrix
  _proj = XMMatrixPerspectiveFovLH(vfov, aspectRatio, 0.01f, 200000.f);
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

	_groundProgram = _renderer->CreateShaderProgram(
      CompileShaderFromFile(L"shaders/PBR_VS.hlsl", "main", "vs_5_0"),
      CompileShaderFromFile(L"shaders/PBR_Ground_PS.hlsl", "main", "ps_5_0"),
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

	_defaultSampler = _renderer->CreateSamplerState(D3D11_FILTER_ANISOTROPIC,
                                                  D3D11_TEXTURE_ADDRESS_WRAP);

  _specularTexture = _renderer->CreateTextureCube("assets/textures/BakerSpecularIBL.dds",
      DXGI_FORMAT_R32G32B32A32_FLOAT, 10);
  _irradianceTexture = _renderer->CreateTextureCube("assets/textures/BakerDiffuseIrradiance.dds",
      DXGI_FORMAT_R32G32B32A32_FLOAT, 1);
  _specularBRDF_LUT = _renderer->CreateTexture("assets/textures/BakerSpecularBRDF_LUT.dds",
      DXGI_FORMAT_R32G32B32A32_FLOAT, 1);
  /*_specularBRDF_LUT = _renderer->CreateTexture("assets/textures/ibl_brdf_lut.dds",
                               DXGI_FORMAT_R32G32_FLOAT, 1);*/

	_spBRDF_Sampler = _renderer->CreateSamplerState(
      D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP);

	// Ground
  _groundATexture = _renderer->CreateTexture(
      Image::fromFile("assets/textures/forrest_ground_A.jpg"),
      DXGI_FORMAT_R8G8B8A8_UNORM);
  _groundNTexture = _renderer->CreateTexture(
      Image::fromFile("assets/textures/forrest_ground_N.png"),
      DXGI_FORMAT_R8G8B8A8_UNORM);
  _groundMTexture = _renderer->CreateTexture(
      Image::fromFile("assets/textures/forrest_ground_M.jpg", 1),
      DXGI_FORMAT_R8_UNORM);
  _groundRTexture = _renderer->CreateTexture(
      Image::fromFile("assets/textures/forrest_ground_R.png", 1),
      DXGI_FORMAT_R8_UNORM);
}

void DemoApp::InitSamplers() {
  _defaultSampler = _renderer->CreateSamplerState(D3D11_FILTER_ANISOTROPIC,
                                                  D3D11_TEXTURE_ADDRESS_WRAP);
  _computeSampler = _renderer->CreateSamplerState(
      D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP);
}

void DemoApp::InitMeshes() {
  _ground =
      _renderer->CreateMeshBuffer(Mesh::fromFile("assets/model/ground.obj"));
  _pbrModel =
      _renderer->CreateMeshBuffer(Mesh::fromFile("assets/model/cerberus.fbx"));
  _skybox =
      _renderer->CreateMeshBuffer(Mesh::fromFile("assets/model/skybox.obj"));

	// Animation
  _model = new Model(_renderer->_device, _renderer->_context,
                     "assets/model/SkinningTest.fbx");

  _animation = new Animation("assets/model/SkinningTest.fbx", _model);
  _animator = new Animator(_animation);
}

void DemoApp::InitLights() {
  // Update shading constant buffer (for pixel shader).
  _shadingConstants.eyePosition = g_camPos;
  _shadingConstants.useIBL = _useIBL;
  _shadingConstants.gamma = 2.2f;

  Light light1, light2, light3;
  light1.direction = XMVector4Normalize(g_sunDir);
  light2.direction = {1.0f, 0.0f, 0.0f, 0.f};
  light3.direction = {-1.0f, 0.0f, 0.0f, 0.f};

  light1.radiance = {5.0f, 5.0f, 5.0f, 1.f};
  light2.radiance = {1.0f, 1.0f, 1.0f, 1.f};
  light3.radiance = {1.0f, 1.0f, 1.0f, 1.f};

  light1.enabled = true;
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
