#pragma once

#include "D3DEngine/EngineCommon.h"

namespace DX {

class D3D11Renderer {
  class RenderDevice* _device{nullptr};
  class SwapChain* _swapchain{nullptr};

  D3D11Renderer(const D3D11Renderer&) = delete;
  D3D11Renderer& operator=(const D3D11Renderer&) = delete;

 public:
  void Initialize(HWND hWnd, UINT width, UINT height, bool allowTearing = false);

  void Shutdown();

	void BeginFrame();
  void BeginDraw();
  void EndDraw();
  void EndFrame();
};

}  // namespace DX