#include "D3DEngine/Renderer/D3D11Renderer.h"

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
  PipelineState* _opaquePipeline;
  PipelineState* _lightPipeline;

	FrameBuffer* _geometryPassFBO;
  FrameBuffer* _lightPassFBO;

  RenderPass _geometryPass;
  RenderPass _lightingPass;

	DX::FrameData _frameData;
	ComPtr<ID3D11Buffer> _frameDataCB;
  DX::ObjectData _objectData;
	ComPtr<ID3D11Buffer> _objectDataCB;

	std::unordered_map<Handle, Handle> meshHandleMap;
	HandleTable<DX::MeshBuffer> meshHandleTable;

  std::unordered_map<Handle, Handle> materialHandleMap;
	HandleTable<DX::MaterialInstance> materialHandleTable;

  std::unordered_map<Handle, Handle> textureHandleMap;
  HandleTable<DX::TextureBuffer> textureHandleTable;

	std::unordered_map<Handle, Handle> vsHandleMap;
	HandleTable<ComPtr<ID3D11VertexShader>> vsHandleTable;
  std::unordered_map<Handle, Handle> psHandleMap;
  HandleTable<ComPtr<ID3D11PixelShader>> psHandleTable;
};

void D3D11Renderer::Initialize(HWND hWnd, UINT width, UINT height, bool allowTearing) {

	_device = RenderDevice::CreateRenderDevice();
  _swapchain = _device->CreateSwapChain(hWnd, width, height, allowTearing);

  _m = new D3D11Renderer::Private;
}

void D3D11Renderer::Shutdown() { 
	delete _m;

	delete _swapchain;
  delete _device;
}

void D3D11Renderer::BeginFrame() { 


	
}

void D3D11Renderer::BeginDraw() {}

void D3D11Renderer::DrawMesh(Handle meshHandle, XMMATRIX transform) {
  const MeshData& meshData = AccessMeshData(meshHandle);

	// Get material
  Handle materialHandle = meshData.material;
  const MaterialData& materialData = AccessMaterialData(materialHandle);



}

void DX::D3D11Renderer::DrawLight(const LightData& light) {
	// TODO:

}

void D3D11Renderer::EndDraw() {}

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

void DX::D3D11Renderer::InitShaders() {
	

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
		.VSSetVertexShader
}

void DX::D3D11Renderer::InitRenderPass() {}
