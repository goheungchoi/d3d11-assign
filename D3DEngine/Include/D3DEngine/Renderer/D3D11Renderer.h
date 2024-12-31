#pragma once

#include "IRenderer.h"

namespace DX {

class D3D11Renderer : public IRenderer {
  class RenderDevice* _device{nullptr};
  class SwapChain* _swapchain{nullptr};

  D3D11Renderer(const D3D11Renderer&) = delete;
  D3D11Renderer& operator=(const D3D11Renderer&) = delete;

 public:
  void Initialize(HWND hWnd, UINT width, UINT height, bool allowTearing = false) override;

  void Shutdown() override;

	void BeginFrame() override;
  void BeginDraw() override;
  void EndDraw() override;
  void EndFrame() override;
};

}  // namespace DX