#pragma once

namespace DX {

class D3D11Renderer {
  class RenderDevice* _device;
  class SwapChain* _swapchain;

  D3D11Renderer(const D3D11Renderer&) = delete;
  D3D11Renderer& operator=(const D3D11Renderer&) = delete;

 public:
  HRESULT Initialize(HWND hWnd, UINT width, UINT height);

  void Shutdown();

  void BeginDraw();
  void EndDraw();
};

}  // namespace DX