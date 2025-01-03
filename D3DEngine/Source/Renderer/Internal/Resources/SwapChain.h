#pragma once

#include "Renderer/Internal/D3D11Common.h"
#include "Renderer/Internal/D3D11Types.h"

namespace DX {

class SwapChain {
  bool _windowed{false};
  bool _allowTearing{false};
  UINT _width, _height;

  class RenderDevice& _device;

  ComPtr<IDXGISwapChain1> _swapchain;

	D3D11_TEXTURE2D_DESC _backbufferDesc{};
  ComPtr<ID3D11RenderTargetView> _backBufferRTV;

  SwapChain(class RenderDevice& device) : _device{device} {}

	friend class RenderDevice;

 public:

	void SetWindowMode(bool windowed);

	void Resize(UINT width = 0, UINT height = 0);

	ID3D11RenderTargetView* GetBackBuffer() { return _backBufferRTV.Get(); }

	HRESULT Present() { 
		HRESULT res;

		if (_allowTearing) {
      // Recommended to always use tearing if supported when using a sync
      // interval of 0.
      res = _swapchain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
    } else {
      // The first argument instructs DXGI to block until VSync, putting the
      // application to sleep until the next VSync. This ensures we don't waste
      // any cycles rendering frames that will never be displayed to the screen.
      res = _swapchain->Present(1, 0);
		}
    
		return res;
	}

 private:
  void CreateDXGISwapChain(HWND hwnd, UINT width, UINT height,
                           bool allowTearing);

  void CreateBackBufferView();

  void Initialize(HWND hwnd, UINT width, UINT height, bool allowTearing) {
    CreateDXGISwapChain(hwnd, width, height, allowTearing);
		CreateBackBufferView();
	}
};

}  // namespace DX
