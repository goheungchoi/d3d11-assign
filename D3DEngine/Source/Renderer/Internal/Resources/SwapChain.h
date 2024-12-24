#pragma once

#include "Renderer/Internal/D3D11Common.h"
#include "Renderer/Internal/D3D11Types.h"

#include "RenderDevice.h"

namespace DX {

class SwapChain {
  bool _windowed{false};

  ComPtr<IDXGISwapChain1> _swapchain;

	ComPtr<ID3D11RenderTargetView> _backBufferRTV;

	SwapChain() = default;

 public:
  SwapChain* CreateSwapChain(HWND hwnd, UINT width, UINT height,
                             RenderDevice* device) {
    SwapChain* swapchain = new SwapChain();
    swapchain->Initialize(hwnd, width, height, device);
		return swapchain;
	}

	void SwitchWindowMode() {
    if (_windowed) {	// To full-screen
      _swapchain->SetFullscreenState(TRUE, NULL);
			// TODO:
			
		} else {	// To windowed
      _swapchain->SetFullscreenState(FALSE, NULL);
			// TODO: 
		}

		_windowed = !_windowed;
	}

	void Resize(UINT width, UINT height) {

	}

 private:
  void CreateDXGISwapChain(HWND hwnd, UINT width, UINT height,
                           RenderDevice* device) {
    bool swapFlip =
        (!device->_vsync || device->_enableHDR || device->_isFlipPresent);

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc{
        .Width = width,
        .Height = height,
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
        .SampleDesc = {.Count = 1},

        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = 2,
        .SwapEffect =
            swapFlip ? DXGI_SWAP_EFFECT_FLIP_DISCARD : DXGI_SWAP_EFFECT_DISCARD,
        .AlphaMode = DXGI_ALPHA_MODE_IGNORE,
        .Flags = (!device->_vsync) ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0,
    };
    swapChainDesc.Flags |=
        DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;  // Allow full-screen switching
  
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc{
        .Windowed = _windowed,
    };

		ThrowIfFailed(device->_dxgiFactory->CreateSwapChainForHwnd(
        device->GetDevice(), hwnd, &swapChainDesc, &fullscreenDesc, nullptr,
        _swapchain.ReleaseAndGetAddressOf()));
	}

  void CreateBackBufferView(RenderDevice* device) { 
		
		ComPtr<ID3D11Texture2D> backBuffer;
    ThrowIfFailed(_swapchain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)));

		device->GetDevice()->CreateRenderTargetView(backBuffer.Get(), nullptr,
                                                &_backBufferRTV);
	}

  void Initialize(HWND hwnd, UINT width, UINT height, RenderDevice* device) {
		
	
	}
};

}  // namespace DX