#include "D3DEngine/Renderer/D3D11Renderer.h"

#include "D3DEngine/EngineCommon.h"

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include "Internal/Resources/RenderDevice.h"
#include "Internal/Resources/RenderContext.h"
#include "Internal/Resources/SwapChain.h"

#include "Internal/Components/RenderPass.h"

using namespace DX;

#include "directxtk/DDSTextureLoader.h"
using namespace DirectX;

#include "D3DEngine/Core/Handle.h"

#include "D3DEngine/ResourceManager/ResourceManager.h"

struct D3D11Renderer::Private {
  DX::RenderContext* _context;

  PipelineState* _opaquePipeline;
  PipelineState* _lightPipeline;

	RenderTargetBuffer _positionRT;
  RenderTargetBuffer _albedoRT;
  RenderTargetBuffer _normalRT;
  RenderTargetBuffer _metalRoughnessRT;
  DepthStensilBuffer _depthBuffer;

	FrameBuffer* _geometryPassFBO;
  FrameBuffer* _lightPassFBO;

  RenderPass _geometryPass;
  RenderPass _lightPass;

	ComPtr<ID3D11SamplerState> _defaultSampler;
	ComPtr<ID3D11SamplerState> _wrapSampler;
  ComPtr<ID3D11SamplerState> _clampSampler;
	ComPtr<ID3D11SamplerState> _pointSampler;

	DX::FrameData _frameData;
	ComPtr<ID3D11Buffer> _frameDataCB;

	HandleTable<DX::MeshBuffer> meshHandleTable;

	HandleTable<DX::MaterialInstance> materialHandleTable;

  HandleTable<DX::TextureBuffer> textureHandleTable;

	HandleTable<DX::LightData> lightHandleTable;

	HandleTable<ComPtr<ID3D11VertexShader>> vsHandleTable;
  HandleTable<ComPtr<ID3D11PixelShader>> psHandleTable;
};

void D3D11Renderer::Initialize(HWND hWnd, UINT width, UINT height, bool allowTearing) {

	_device = RenderDevice::CreateRenderDevice();
  _swapchain = _device->CreateSwapChain(hWnd, width, height, allowTearing);

  _m = new D3D11Renderer::Private;

	_m->_context = _device->CreateRenderContext();

	InitShaders();
  InitPipelineState();
  InitRenderPass();
	InitImGui();
}

void D3D11Renderer::Shutdown() { 
	delete _m;

	delete _swapchain;
  delete _device;
}

void DX::D3D11Renderer::BeginFrame(XMMATRIX view, XMMATRIX proj) {
  _m->_frameData.view = XMMatrixTranspose(view);
  _m->_frameData.invView = XMMatrixTranspose(XMMatrixInverse(nullptr, view));
  _m->_frameData.proj = XMMatrixTranspose(proj);
  _m->_frameData.invProj = XMMatrixTranspose(XMMatrixInverse(nullptr, proj));
  _m->_frameData.viewProj = XMMatrixTranspose(XMMatrixMultiply(view, proj));


}

void D3D11Renderer::BeginDraw() { 
	_m->_context->StartCommandList(); 
}

void D3D11Renderer::DrawMesh(Handle meshHandle, XMMATRIX transform) {
  const MeshData& meshData = AccessMeshData(meshHandle);

	// Get material
  Handle materialHandle = meshData.material;
  const MaterialData& materialData = AccessMaterialData(materialHandle);



}

void DX::D3D11Renderer::DrawLight(Handle lightHandle) {}

void DX::D3D11Renderer::DrawImGui() {}


void D3D11Renderer::EndDraw() { 
	_m->_context->FinishCommandList();
  _device->SubmitCommandList(_m->_context);
}

void D3D11Renderer::EndFrame() {}

Handle DX::D3D11Renderer::CreateShader(Handle shaderHandle) { 
	// TODO:
	return Handle(); 
}

Handle D3D11Renderer::CreateTexture(Handle textureHandle) {
  // TODO:
  return Handle(); 
}

Handle DX::D3D11Renderer::CreateMesh(Handle meshHandle) { 	
	const MeshData& meshData = AccessMeshData(meshHandle);


	// Get material data
	Handle materialHandle = meshData.material;
	const MaterialData& materialData = AccessMaterialData(materialHandle);

  materialData.albedoFactor;
  materialData.albedoTexture;

  materialData.metallicFactor;
  materialData.roughnessFactor;
  materialData.metallicRoughnessTexture;

  materialData.normalTexture;

  materialData.occlusionTexture;

  materialData.emissiveFactor;
  materialData.emissiveTexture;

  materialData.alphaMode;
  materialData.alphaCutoff;
  materialData.doubleSided;

	// TODO:
  if (materialData.alphaMode == AlphaMode::kBlend) {
		// Pass type transparent
  } else {
		// Pass type opaque
	}

	return Handle(); 
}

Handle DX::D3D11Renderer::CreateLight(const DX::LightData* light) {
  return Handle();
}

void DX::D3D11Renderer::InitShaders() {
	

}

void DX::D3D11Renderer::InitSamplers() {
  // Default sampler
  _m->_defaultSampler = _device->CreateSamplerState(D3D11_FILTER_ANISOTROPIC,
                                                    D3D11_TEXTURE_ADDRESS_WRAP);
  // Wrap sampler
  _m->_wrapSampler = _device->CreateSamplerState(
      D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP);

  // Clamp sampler
  _m->_clampSampler = _device->CreateSamplerState(
      D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP);

  // Point sampler
  _m->_pointSampler = _device->CreateSamplerState(
      D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP);
}

void DX::D3D11Renderer::InitConstantBuffers() {
  _m->_frameDataCB = _device->CreateConstantBuffer<DX::FrameData>();
}

void DX::D3D11Renderer::InitPipelineState() {

	Handle geometryVS = LoadShader("Geometry_VS.hlsl", ShaderType::kVertex);
	Handle geometryPS = LoadShader("Geometry_PS.hlsl", ShaderType::kPixel);
	Handle lightVS = LoadShader("Light_VS.hlsl", ShaderType::kVertex);
	Handle lightPS = LoadShader("Light_PS.hlsl", ShaderType::kPixel);

	// TODO:
	DX::PipelineStateBuilder builder;
  _m->_opaquePipeline =
      builder.IAInputTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
          .IAInputLayout(-1)
          .VSSetVertexShader(geometryVS)
          .RSSetViewport(0, 0, _swapchain->GetWidth(), _swapchain->GetHeight())
          .RSSetFillMode(D3D11_FILL_SOLID)
          .RSSetCullMode(D3D11_CULL_BACK)
          .RSDisableMultisample()
          .PSSetPixelShader(geometryPS)
          .OMEnableDepthTesting(D3D11_COMPARISON_GREATER)
          .OMDisableBlending()
          .OMSetColorAttachmentFormats(
              {DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT,
               DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32_FLOAT})
          .OMSetDepthAttachmentFormat(DXGI_FORMAT_D32_FLOAT)
          .Build(_device);

	builder.Reset();
	_m->_lightPipeline =
      builder.IAInputTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
          .IAInputLayout(-1)
          .VSSetVertexShader(lightVS)
          .RSSetViewport(0, 0, _swapchain->GetWidth(), _swapchain->GetHeight())
          .RSSetFillMode(D3D11_FILL_SOLID)
          .RSSetCullMode(D3D11_CULL_FRONT)
          .RSDisableMultisample()
          .PSSetPixelShader(lightPS)
          .OMDisableDepthTesting()
          .OMEnableAdditiveBlending()
          .OMSetColorAttachmentFormats(
              {DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT,
               DXGI_FORMAT_R32G32B32A32_FLOAT, DXGI_FORMAT_R32G32_FLOAT})
          .OMSetDepthAttachmentFormat(DXGI_FORMAT_D32_FLOAT)
          .Build(_device);
}

void DX::D3D11Renderer::InitRenderPass() {

	_m->_positionRT = _device->CreateRenderTargetBuffer(
      _swapchain->GetWidth(), _swapchain->GetHeight(),
      DXGI_FORMAT_R32G32B32A32_FLOAT);

	_m->_albedoRT = _device->CreateRenderTargetBuffer(
      _swapchain->GetWidth(), _swapchain->GetHeight(),
      DXGI_FORMAT_R32G32B32A32_FLOAT);

	_m->_normalRT = _device->CreateRenderTargetBuffer(
      _swapchain->GetWidth(), _swapchain->GetHeight(),
      DXGI_FORMAT_R32G32B32A32_FLOAT);

	_m->_metalRoughnessRT = _device->CreateRenderTargetBuffer(
      _swapchain->GetWidth(), _swapchain->GetHeight(),
      DXGI_FORMAT_R32G32_FLOAT);

	_m->_depthBuffer = _device->CreateDepthStencilBuffer(
      _swapchain->GetWidth(), _swapchain->GetHeight(), DXGI_FORMAT_D32_FLOAT);

  _m->_geometryPassFBO = _device->CreateFrameBuffer(
      _swapchain->GetWidth(), _swapchain->GetHeight(), 1,
      {_m->_positionRT, _m->_albedoRT, _m->_normalRT, _m->_metalRoughnessRT},
      _m->_depthBuffer);

}

void DX::D3D11Renderer::InitImGui() {
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
  ImGui_ImplWin32_Init(_swapchain->GetWindowHandle());
  ImGui_ImplDX11_Init(_device->GetDevice(), _device->GetImmediateContext());
}
