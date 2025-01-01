#pragma once

#include "IRenderer.h"

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

	void BeginFrame() override;
  void BeginDraw() override;

	// Resource bindings
  void DrawMesh(Handle meshHandle, XMMATRIX transform) override;

  void EndDraw() override;
  void EndFrame() override;

	// Resource management
  Handle CreateTexture(Handle textureHandle) override;
  Handle CreateMesh(Handle meshHandle) override;

 private:
  struct Private;
  std::unique_ptr<Private> _m;
};

}  // namespace DX