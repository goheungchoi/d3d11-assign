#pragma once

#include "IRenderer.h"

//struct D3D11Renderer::Private {
//  PipelineState* _opaquePipeline;
//  PipelineState* _lightPipeline;
//
//  FrameBuffer* _geometryPassFBO;
//  FrameBuffer* _lightPassFBO;
//
//  RenderPass _geometryPass;
//  RenderPass _lightingPass;
//
//  DX::FrameData _frameData;
//  ComPtr<ID3D11Buffer> _frameDataCB;
//  DX::ObjectData _objectData;
//  ComPtr<ID3D11Buffer> _objectDataCB;
//
//  std::unordered_map<Handle, Handle> meshHandleMap;
//  HandleTable<DX::MeshBuffer> meshHandleTable;
//
//  std::unordered_map<Handle, Handle> materialHandleMap;
//  HandleTable<DX::MaterialInstance> materialHandleTable;
//
//  std::unordered_map<Handle, Handle> textureHandleMap;
//  HandleTable<DX::TextureBuffer> textureHandleTable;
//
//  std::unordered_map<Handle, Handle> vsHandleMap;
//  HandleTable<ComPtr<ID3D11VertexShader>> vsHandleTable;
//  std::unordered_map<Handle, Handle> psHandleMap;
//  HandleTable<ComPtr<ID3D11PixelShader>> psHandleTable;
//};

namespace DX {

class D3D11Renderer : public IRenderer {
  class RenderDevice* _device{nullptr};
  class SwapChain* _swapchain{nullptr};

  D3D11Renderer(const D3D11Renderer&) = delete;
  D3D11Renderer& operator=(const D3D11Renderer&) = delete;

 public:
  D3D11Renderer() = default;

  void Initialize(HWND hWnd, UINT width, UINT height, bool allowTearing = false) override;

  void Shutdown() override;

	void BeginFrame(XMMATRIX view, XMMATRIX proj) override;
  void BeginDraw() override;

	// Resource bindings
  void DrawMesh(Handle renderMeshHandle, XMMATRIX transform) override;
  void DrawLight(Handle lightHandle) override;
  void DrawImGui() override;

  void EndDraw() override;
  void EndFrame() override;

	// Resource management
  Handle CreateShader(Handle shaderHandle) override;
  Handle CreateTexture(Handle textureHandle) override;
  Handle CreateMesh(Handle meshHandle) override;
	Handle CreateLight(const LightData* light) override;

 private:
  struct Private;
  Private* _m;

	void InitShaders();
  void InitSamplers();
  void InitConstantBuffers();
	void InitPipelineState();
  void InitRenderPass();
  void InitImGui();
};

}  // namespace DX