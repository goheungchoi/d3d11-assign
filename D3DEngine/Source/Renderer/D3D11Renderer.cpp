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

// TODO: Manage pipeline state with unordered_multimap or others...
static PipelineState* g_prevPipelineState{nullptr};

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

	CubeTextureBuffer _environmentMap;
	CubeTextureBuffer _specularCubeTexture;
  CubeTextureBuffer _irradianceCubeTexture;
  TextureBuffer _specularBRDF_LUT_Texture;

	ComPtr<ID3D11SamplerState> _defaultSampler;
	ComPtr<ID3D11SamplerState> _wrapSampler;
  ComPtr<ID3D11SamplerState> _clampSampler;
	ComPtr<ID3D11SamplerState> _pointSampler;

	DX::cbFrameData _frameData;
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

	InitSamplers();
  InitConstantBuffers();
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

	_device->CopyMappedData(_m->_frameDataCB, &_m->_frameData);
}

void D3D11Renderer::BeginDraw() { 
	_m->_context->StartCommandList(); 

	_m->_context->BeginRendering(_m->_geometryPass);
}

void D3D11Renderer::DrawMesh(Handle meshBufHandle, XMMATRIX transform) {
  auto& meshBuf = _m->meshHandleTable[meshBufHandle];
  if (meshBuf) {
    auto& matInstance = _m->materialHandleTable[meshBuf->materialInstance];
		
		if (matInstance) {
      DX::cbObjectData objData{.world = XMMatrixTranspose(transform)};
      _device->CopyMappedData(matInstance->cbSet[1], &objData);

			// TODO: Use this state change prevention just for now.
      if (g_prevPipelineState != matInstance->pipeline) {
				_m->_context->BindPipelineState(matInstance->pipeline);
        g_prevPipelineState = matInstance->pipeline;
			}
			_m->_context->DrawMeshBuffer(meshBuf.value(), matInstance.value());
		}
	}
}

void DX::D3D11Renderer::DrawLight(Handle lightHandle) {}

void DX::D3D11Renderer::DrawImGui() {
	// TODO: Move the back buffer into a separate frame buffer
  ID3D11RenderTargetView* rtv[] = {_swapchain->GetBackBuffer()};
  _device->GetImmediateContext()->OMSetRenderTargets(1, rtv, nullptr);
  const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
  _device->GetImmediateContext()->ClearRenderTargetView(*rtv, clearColor);

  // Start the Dear ImGui frame
  ImGui_ImplDX11_NewFrame();
  ImGui_ImplWin32_NewFrame();
  ImGui::NewFrame();

  if (ImGui::Begin("Properties")) {
    ImGui::Text("Position: ");
    ImTextureID positionRT_ID = (ImTextureID)(uintptr_t)_m->_positionRT.srv.Get();
    ImGui::Image(positionRT_ID, ImVec2(400, 400));

		ImGui::Text("Albedo: ");
		ImTextureID albedoRT_ID = (ImTextureID)(uintptr_t)_m->_albedoRT.srv.Get();
    ImGui::Image(albedoRT_ID, ImVec2(400, 400));

		ImGui::Text("Normal: ");
		ImTextureID normalRT_ID = (ImTextureID)(uintptr_t)_m->_normalRT.srv.Get();
    ImGui::Image(normalRT_ID, ImVec2(400, 400));

		ImGui::Text("MetalRoughness: ");
		ImTextureID metalRoughnessRT_ID = (ImTextureID)(uintptr_t)_m->_metalRoughnessRT.srv.Get();
    ImGui::Image(metalRoughnessRT_ID, ImVec2(400, 400));

		ImGui::Text("Depth: ");
		ImTextureID deptRT_ID = (ImTextureID)(uintptr_t)_m->_depthBuffer.srv.Get();
    ImGui::Image(deptRT_ID, ImVec2(400, 400));
  }
  ImGui::End();

  // Rendering
  ImGui::Render();
  ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}


void D3D11Renderer::EndDraw() { 
	_m->_context->FinishCommandList();
  _device->SubmitCommandList(_m->_context);

	g_prevPipelineState = nullptr;
}

void D3D11Renderer::EndFrame() { 
	_swapchain->Present(); 
}

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

	DX::MeshBuffer meshBuf = _device->CreateMeshBuffer(meshData);

	// Create a mat instance
  MaterialInstance matInstance{};

	// Set the sampler states
  matInstance.samplerSet.push_back(_m->_defaultSampler);
  matInstance.samplerSet.push_back(_m->_wrapSampler);
  matInstance.samplerSet.push_back(_m->_clampSampler);
  matInstance.samplerSet.push_back(_m->_pointSampler);
	
	// Add the frame data constant buffer
  matInstance.cbSet.push_back(_m->_frameDataCB);

	// Add the object data constant buffer
  ComPtr<ID3D11Buffer> _objectDataCB = _device->CreateConstantBuffer<DX::cbObjectData>();
  matInstance.cbSet.push_back(_objectDataCB);

	// Set material instance
	if (meshData.material != Handle::kInvalidHandle) {
    // Get material data
    Handle materialHandle = meshData.material;
    const MaterialData& materialData = AccessMaterialData(materialHandle);

		// Set pass type and pipeline.
    if (materialData.alphaMode == AlphaMode::kBlend) {
      matInstance.passType = MaterialPass::kTransparent;
    } else {
      matInstance.passType = MaterialPass::kOpaque;
      matInstance.pipeline = _m->_opaquePipeline;
    }

		// Create texture data
		const auto& albedoTextureData = DX::AccessTextureData(materialData.albedoTexture);
    DX::TextureBuffer albedoTexBuf = _device->CreateTextureBuffer(albedoTextureData);
    matInstance.textureSet.push_back(albedoTexBuf);
		
		if (!materialData.metallicRoughnessTexture.IsInvalid()) {
      const auto& metalRoughnessTextureData =
          DX::AccessTextureData(materialData.metallicRoughnessTexture);
      DX::TextureBuffer metalRoughnessTexBuf =
          _device->CreateTextureBuffer(albedoTextureData);
      matInstance.textureSet.push_back(metalRoughnessTexBuf);
		}
		
		if (!materialData.normalTexture.IsInvalid()) {
      const auto& normalTextureData =
          DX::AccessTextureData(materialData.normalTexture);
      DX::TextureBuffer normalTexBuf =
          _device->CreateTextureBuffer(normalTextureData);
      matInstance.textureSet.push_back(normalTexBuf);
		}
		
		// Create a shading mat constant buffer
		cbShadingMaterial shadingMat{
        .albedoFactor = materialData.albedoFactor,
        .metallicFactor = materialData.metallicFactor,
        .roughnessFactor = materialData.roughnessFactor,
        .alphaCutoff = materialData.alphaCutoff};

		auto cbo = _device->CreateConstantBuffer<cbShadingMaterial>(D3D11_USAGE_DEFAULT, &shadingMat);
    matInstance.cbSet.push_back(cbo);
	}

	// Attach the material handle to the mesh
  meshBuf.materialInstance =
      _m->materialHandleTable.ClaimHandle(std::move(matInstance));

	// Create the mesh buffer handle.
  Handle meshBufHandle = _m->meshHandleTable.ClaimHandle(std::move(meshBuf));

	return meshBufHandle;
}

Handle DX::D3D11Renderer::CreateLight(const DX::LightData* light) {
  return Handle();
}

void DX::D3D11Renderer::InitEnvMap() {
	

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
  _m->_frameDataCB = _device->CreateConstantBuffer<DX::cbFrameData>();
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
          .OMSetColorAttachmentFormats({DXGI_FORMAT_R32G32B32A32_FLOAT})
          .OMSetDepthAttachmentFormat(DXGI_FORMAT_D32_FLOAT)
          .Build(_device);
}

void DX::D3D11Renderer::InitRenderPass() {

	// Geometry pass
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

	_m->_geometryPass.BindFrameBuffer(_m->_geometryPassFBO);
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
