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

namespace {
auto CalculatePointLightRadius = [](float intensity, float constant, float linear,
                                    float quadratic, float threshold = 0.001f) {
  float intensityTarget = intensity / threshold;  // I_0 / threshold
  float a = quadratic;
  float b = linear;
  float c = constant - intensityTarget;

  if (quadratic == 0.0f) {
    if (linear == 0.0f) {
      return (constant >= intensityTarget)
                 ? INFINITY
                 : 0.0f;  // Infinite range if no attenuation
    }
    return (intensityTarget - constant) / linear;  // Solve linear case
  }

  // Solve the quadratic equation
  float discriminant = b * b - 4 * a * c;
  if (discriminant < 0.0f) return 0.0f;         // No real solution
  return (-b + sqrtf(discriminant)) / (2 * a);  // Positive root
};
}

struct D3D11Renderer::Private {
  DX::RenderContext* _context;

  PipelineState* _opaquePipeline;
  PipelineState* _pointShadowPipeline;
  PipelineState* _lightPipeline;

	RenderTargetBuffer _positionRT;
  RenderTargetBuffer _albedoRT;
  RenderTargetBuffer _normalRT;
  RenderTargetBuffer _metalRoughnessRT;
  DepthStencilBuffer _depthBuffer;

	RenderTargetBuffer _shadingRT;
  DepthStencilBuffer _shadingDS;

	CubeRenderTargetBuffer _pointShadowRT;
	CubeDepthStencilBuffer _pointShadowDepthBuffer;

	FrameBuffer* _geometryPassFBO;
  FrameBuffer* _pointShadowPassFBO;
  FrameBuffer* _lightPassFBO;

  RenderPass _geometryPass;
  RenderPass _pointShadowPass;
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

	DX::MeshBuffer lightSphere;

	HandleTable<DX::MeshBuffer> meshHandleTable;
	HandleTable<DX::MaterialInstance> materialHandleTable;
  HandleTable<DX::TextureBuffer> textureHandleTable;

	HandleTable<DX::PointLightInstance> lightHandleTable;

	HandleTable<ComPtr<ID3D11VertexShader>> vsHandleTable;
  HandleTable<ComPtr<ID3D11PixelShader>> psHandleTable;


	std::vector<std::pair<Handle, XMMATRIX>> scheduledMeshes;
  std::vector<std::pair<Handle, DX::LightData>> scheduledLights;


	// hdr to ldr compute shader
  TextureBuffer stagingTextureBuffer;
  ComPtr<ID3D11ComputeShader> hdr2ldrCS;
};

void D3D11Renderer::Initialize(HWND hWnd, UINT width, UINT height, bool allowTearing) {

	_device = RenderDevice::CreateRenderDevice();
  _swapchain = _device->CreateSwapChain(hWnd, width, height, allowTearing);

  _m = new D3D11Renderer::Private;

	_m->_context = _device->CreateRenderContext();

	InitEnvMap();
  InitShaders();
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

static XMVECTOR g_eyePosition{};

void DX::D3D11Renderer::BeginFrame(XMVECTOR eyePosition, XMMATRIX view, XMMATRIX proj) {
  g_eyePosition = eyePosition;

  _m->_frameData.view = XMMatrixTranspose(view);
  _m->_frameData.invView = XMMatrixTranspose(XMMatrixInverse(nullptr, view));
  _m->_frameData.proj = XMMatrixTranspose(proj);
  _m->_frameData.invProj = XMMatrixTranspose(XMMatrixInverse(nullptr, proj));
  _m->_frameData.viewProj = XMMatrixTranspose(XMMatrixMultiply(view, proj));

	_device->CopyMappedData(_m->_frameDataCB, &_m->_frameData);
}

void D3D11Renderer::BeginDraw() { 
	_m->scheduledMeshes.clear();
  _m->scheduledLights.clear();

	_m->_context->StartCommandList(); 
}

// TODO: Separate the material binding.
void DX::D3D11Renderer::ScheduleMesh(Handle meshBufHandle, XMMATRIX transform) {
  if (_m->meshHandleTable.IsValidHandle(meshBufHandle)) {
    _m->scheduledMeshes.push_back({meshBufHandle, transform});
  }
}

void DX::D3D11Renderer::ScheduleLight(Handle lightHandle,
                                      const DX::LightData* data) {
  if (_m->lightHandleTable.IsValidHandle(lightHandle)) {
    _m->scheduledLights.push_back({lightHandle, *data});
	}
}

void DX::D3D11Renderer::DrawOpaqueMeshes() {
  _m->_context->BeginRendering(_m->_geometryPass);

  for (auto& [meshBufHandle, transform] : _m->scheduledMeshes) {
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

	_m->_context->EndRendering();
}

void DX::D3D11Renderer::DrawShadows() {
	_m->_context->BindPipelineState(_m->_pointShadowPipeline);

	for (auto& [handle, lightData] : _m->scheduledLights) {
    auto& lightInstance = _m->lightHandleTable[handle];
		
		if (lightInstance) {
      XMVECTOR lightPosition = lightData.components;

			const XMMATRIX viewTransforms[6] = {
          XMMatrixTranspose(XMMatrixLookAtLH(
              lightPosition, lightPosition + XMVectorSet(1, 0, 0, 0),
              XMVectorSet(0, 1, 0, 0))),  // +X
          XMMatrixTranspose(XMMatrixLookAtLH(
              lightPosition, lightPosition + XMVectorSet(-1, 0, 0, 0),
              XMVectorSet(0, 1, 0, 0))),  // -X
          XMMatrixTranspose(XMMatrixLookAtLH(
              lightPosition, lightPosition + XMVectorSet(0, 1, 0, 0),
              XMVectorSet(0, 0, -1, 0))),  // +Y
          XMMatrixTranspose(XMMatrixLookAtLH(
              lightPosition, lightPosition + XMVectorSet(0, -1, 0, 0),
              XMVectorSet(0, 0, 1, 0))),  // -Y
          XMMatrixTranspose(XMMatrixLookAtLH(
              lightPosition, lightPosition + XMVectorSet(0, 0, 1, 0),
              XMVectorSet(0, 1, 0, 0))),  // +Z
          XMMatrixTranspose(XMMatrixLookAtLH(
              lightPosition, lightPosition + XMVectorSet(0, 0, -1, 0),
              XMVectorSet(0, 1, 0, 0)))  // -Z
      };

			for (int i = 0; i < 6; ++i) {
        // Get the view transforms
        cbPointLightTransform transform{
            .view = viewTransforms[i],
            .proj = XMMatrixTranspose(
                XMMatrixPerspectiveFovLH(XM_PIDIV2, lightData.nearPlane, lightData.farPlane, 1.f))};
        _device->CopyMappedData(lightInstance->pointLightTransforms[i],
                                &transform);

        cbCameraData camData{lightData.nearPlane, lightData.farPlane};
        _device->CopyMappedData(lightInstance->cameraData, &camData);

        // Create the frame buffer for the shadow map.
        DX::FrameBuffer frameBuf{1024, 1024, 1};

        // Attach render target
        auto& rt = lightInstance->renderTarget;
        AttachmentInfo colorAttachment{.view = rt.rtvs[i],
                                       .bind = D3D11_BIND_RENDER_TARGET,
                                       .format = rt.format,
                                       .width = rt.width,
                                       .height = rt.height,
                                       .samples = 1};
        frameBuf.SetColorAttachment(0, colorAttachment);

        // Attach depth buffer
        auto dp = lightInstance->shadowMap;
        AttachmentInfo depthAttachment{.view = dp.dsvs[i],
                                       .bind = D3D11_BIND_DEPTH_STENCIL,
                                       .format = dp.format,
                                       .width = dp.width,
                                       .height = dp.height,
                                       .samples = 1};
        frameBuf.SetDepthStencilAttachment(depthAttachment);

        // Bind it to the shadow pass.
        _m->_pointShadowPass.BindFrameBuffer(&frameBuf);
        _m->_context->BeginRendering(_m->_pointShadowPass);

        for (auto& [meshBufHandle, transform] : _m->scheduledMeshes) {
          auto& meshBuf = _m->meshHandleTable[meshBufHandle];
          if (meshBuf) {
            _m->_context->DrawPointLightShadow(meshBuf.value(),
                                               lightInstance.value(), i);
          }
        }

        _m->_context->EndRendering();
			}
    }
	}
}

void DX::D3D11Renderer::DrawLights() {
  _m->_context->BeginRendering(_m->_lightPass);
  _m->_context->BindPipelineState(_m->_lightPipeline);

	ID3D11ShaderResourceView* srvs[9] = {
      _m->_positionRT.srv.Get(),
      _m->_albedoRT.srv.Get(),
      _m->_normalRT.srv.Get(),
      _m->_metalRoughnessRT.srv.Get(),
      _m->_depthBuffer.srv.Get(),
      _m->_specularCubeTexture.srv.Get(),
      _m->_irradianceCubeTexture.srv.Get(),
      _m->_specularBRDF_LUT_Texture.srv.Get()};

	_m->_context->Get()->PSSetShaderResources(0, 8, srvs);

	ID3D11SamplerState* samplers[4] = {
      _m->_defaultSampler.Get(), _m->_wrapSampler.Get(),
      _m->_clampSampler.Get(), _m->_pointSampler.Get()};
  _m->_context->Get()->PSSetSamplers(0, 4, samplers);

  for (auto& [handle, lightData] : _m->scheduledLights) {
    auto& lightInstance = _m->lightHandleTable[handle];

    if (lightInstance) {
      XMVECTOR lightPosition = lightData.components;

			float radius = lightData.farPlane;
      // Get the light world transform
      XMMATRIX scale = XMMatrixScaling(radius, radius, radius);
      XMMATRIX translate = XMMatrixTranslationFromVector(lightData.components);
      cbObjectData objData{.world = XMMatrixTranspose(scale * translate)};
      _device->CopyMappedData(lightInstance->objWorld, &objData);

      // Get the light shading constants
      cbLightShadingConstants lightShadingConstants{
          lightData,
          g_eyePosition,
          1.f,
          2.2,
          TRUE,
          TRUE,
          (float)_swapchain->GetWidth(),
          (float)_swapchain->GetHeight()};
      _device->CopyMappedData(lightInstance->lightShadingConstant,
                              &lightShadingConstants);

			_m->_context->Get()->PSSetShaderResources(
          8, 1, lightInstance->shadowMap.srv.GetAddressOf());
			_m->_context->DrawPointLightShading(_m->lightSphere, lightInstance.value());
    }
  }

  _m->_context->EndRendering();

	// HDR to LDR
	_m->_context->Get()->CSSetShader(_m->hdr2ldrCS.Get(), nullptr, 0);
  _m->_context->Get()->CSSetShaderResources(0, 1, _m->_shadingRT.srv.GetAddressOf());
  _m->_context->Get()->CSSetUnorderedAccessViews(
      0, 1, _m->stagingTextureBuffer.uav.GetAddressOf(), nullptr);
  _m->_context->Get()->Dispatch(_swapchain->GetWidth() / 16,
                                _swapchain->GetHeight() / 16, 1);

	ID3D11RenderTargetView* rtv[] = {_swapchain->GetBackBufferRTV()};
	const float clearColor[4] = {0.0f, 0.0f, 0.0f, 1.0f};
	_m->_context->Get()->ClearRenderTargetView(*rtv, clearColor);
	_m->_context->Get()->CopyResource(_swapchain->GetBackBuffer(),
                                    _m->stagingTextureBuffer.texture.Get());
}

void DX::D3D11Renderer::DrawImGui() {
	// TODO: Move the back buffer into a separate frame buffer
  ID3D11RenderTargetView* rtv[] = {_swapchain->GetBackBufferRTV()};
  _m->_context->Get()->OMSetRenderTargets(1, rtv, nullptr);

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

Handle DX::D3D11Renderer::CreateLight(uint32_t type) {
  DX::LightType lightType = (DX::LightType)type;
  if (lightType == DX::LightType::kPoint) {
    PointLightInstance light;
    light.renderTarget = _device->CreateCubeRenderTargetBuffer(
        1024, 1024, DXGI_FORMAT_R8G8B8A8_UINT);
    light.shadowMap = _device->CreateCubeDepthStencilBuffer(
        1024, 1024, DXGI_FORMAT_D32_FLOAT);

		light.pointLightTransforms.resize(6);
    for (int i = 0; i<6; ++i)
      light.pointLightTransforms[i] =
          _device->CreateConstantBuffer<cbPointLightTransform>();
    light.cameraData = _device->CreateConstantBuffer<cbCameraData>();

		light.frameData = _m->_frameDataCB;
    light.objWorld = _device->CreateConstantBuffer<cbObjectData>();
    light.lightShadingConstant = _device->CreateConstantBuffer<cbLightShadingConstants>();

		return _m->lightHandleTable.ClaimHandle(std::move(light));
	}

  return Handle();
}

void DX::D3D11Renderer::InitEnvMap() {
  Handle envHandle = DX::LoadTexture("Textures/BakerEnv.dds", DX::TextureType::kAlbedo);
  const auto& envData = DX::AccessTextureData(envHandle);
  _m->_environmentMap = _device->CreateCubeTextureBuffer(envData);

  Handle spBRDF_LUT_Handle = DX::LoadTexture("Textures/BakerSpecularBRDF_LUT.dds",
                  DX::TextureType::kAlbedo);
  const auto& spBRDF_LUT_Data = DX::AccessTextureData(spBRDF_LUT_Handle);
  _m->_specularBRDF_LUT_Texture = _device->CreateTextureBuffer(spBRDF_LUT_Data);

  Handle diffIrradianceHandle = DX::LoadTexture("Textures/BakerDiffuseIrradiance.dds",
                  DX::TextureType::kAlbedo);
  const auto& diffIrradianceData = DX::AccessTextureData(diffIrradianceHandle);
  _m->_irradianceCubeTexture = _device->CreateCubeTextureBuffer(diffIrradianceData);

  Handle specularIBLHandle = DX::LoadTexture("Textures/BakerSpecularIBL.dds", DX::TextureType::kAlbedo);
  const auto& specularIBLData = DX::AccessTextureData(specularIBLHandle);
  _m->_specularCubeTexture = _device->CreateCubeTextureBuffer(specularIBLData);
}

void DX::D3D11Renderer::InitShaders() {
  Handle hdr2ldrCS = LoadShader("HDR2LDR_CS.hlsl", ShaderType::kCompute);
  const DX::ShaderData& csData = AccessShaderData(hdr2ldrCS);

	_device->GetDevice()->CreateComputeShader(csData.data.data(),
                                            csData.data.size(), nullptr,
                                            _m->hdr2ldrCS.GetAddressOf());

	_m->stagingTextureBuffer = _device->CreateTextureBuffer(
      _swapchain->GetWidth(), _swapchain->GetHeight(),
      DXGI_FORMAT_R8G8B8A8_UINT, D3D11_BIND_UNORDERED_ACCESS, 1);

	_m->stagingTextureBuffer.uav =
      _device->CreateTextureUAV(_m->stagingTextureBuffer.texture.Get(), 0);
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

  Handle pointShadowMapVS =
      LoadShader("PointShadowMap_VS.hlsl", ShaderType::kVertex);
  Handle pointShadowMapPS =
      LoadShader("PointShadowMap_PS.hlsl", ShaderType::kPixel);

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

	_m->_pointShadowPipeline =
      builder.IAInputTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
          .IAInputLayout(0)
          .VSSetVertexShader(pointShadowMapVS)
          .RSSetViewport(0, 0, 1024, 1024)
          .RSSetFillMode(D3D11_FILL_SOLID)
          .RSSetCullMode(D3D11_CULL_BACK)
          .RSDisableMultisample()
          .PSSetPixelShader(pointShadowMapPS)
          .OMEnableDepthTesting(D3D11_COMPARISON_GREATER)
          .OMDisableBlending()
          .OMSetColorAttachmentFormats({DXGI_FORMAT_R8G8B8A8_UINT})
          .OMSetDepthAttachmentFormat(DXGI_FORMAT_D32_FLOAT)
          .Build(_device);

	builder.Reset();

	_m->_lightPipeline =
      builder.IAInputTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
          .IAInputLayout(-1)
          .VSSetVertexShader(lightVS)
          .RSSetViewport(0, 0, _swapchain->GetWidth(), _swapchain->GetHeight())
          .RSSetFillMode(D3D11_FILL_SOLID)
          .RSSetCullMode(D3D11_CULL_BACK)
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

	// Shadow pass



	// Light pass

	// Create the point light mesh
  Handle lightSphereHandle = DX::LoadMesh("Models\\Sphere\\Sphere0.mesh");
  const auto& lightSphereMesh = DX::AccessMeshData(lightSphereHandle);
  _m->lightSphere = _device->CreateMeshBuffer(lightSphereMesh);

  _m->_shadingRT = _device->CreateRenderTargetBuffer(
      _swapchain->GetWidth(), _swapchain->GetHeight(),
      DXGI_FORMAT_R32G32B32A32_FLOAT);

	_m->_shadingDS = _device->CreateDepthStencilBuffer(
      _swapchain->GetWidth(), _swapchain->GetHeight(), DXGI_FORMAT_D16_UNORM);

  _m->_lightPassFBO = _device->CreateFrameBuffer(
      _swapchain->GetWidth(), _swapchain->GetHeight(), 1, {_m->_shadingRT});

	_m->_lightPass.BindFrameBuffer(_m->_lightPassFBO);
	
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
  ImGui_ImplDX11_Init(_device->GetDevice(), _m->_context->Get());
}

