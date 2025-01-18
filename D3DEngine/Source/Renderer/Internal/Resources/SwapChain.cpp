#include "SwapChain.h"

#include "RenderDevice.h"

using namespace DX;

void SwapChain::SetWindowMode(bool windowed) {
  if (windowed) {  // To full-screen
    _swapchain->SetFullscreenState(TRUE, NULL);
    // TODO:

  } else {  // To windowed
    _swapchain->SetFullscreenState(FALSE, NULL);
    // TODO:
  }

  _windowed = windowed;
}

void SwapChain::Resize(UINT width, UINT height) {
  // Unbind the render target.
  _device.GetImmediateContext()->OMSetRenderTargets(0, nullptr, nullptr);

  _backBufferRTV->Release();

  HRESULT hr;
  // Preserve the existing buffer count and format.
  // Automatically choose the width and height to match the client rect for
  // the HWND.
  hr = _swapchain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);

  ThrowIfFailed(hr);

  // Create the back buffer view again
  CreateBackBufferView();
}

void SwapChain::CreateDXGISwapChain(HWND hwnd, UINT width, UINT height,
                         bool allowTearing) {
  _hwnd = hwnd;
  _allowTearing = allowTearing;

  DXGI_SWAP_CHAIN_DESC1 swapChainDesc{
      .Width = width,
      .Height = height,
      .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
      .SampleDesc = {.Count = 1},

      .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
      .BufferCount = 2,
      .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
      .AlphaMode = DXGI_ALPHA_MODE_IGNORE,
      .Flags = allowTearing ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0U,
  };
  swapChainDesc.Flags |=
      DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;  // Allow full-screen switching

  DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc{
      .Windowed = _windowed,
  };

  ThrowIfFailed(_device.GetDXGIFactory()->CreateSwapChainForHwnd(
      _device.GetDevice(), hwnd, &swapChainDesc, &fullscreenDesc, nullptr,
      _swapchain.ReleaseAndGetAddressOf()));

  // Fullscreen and disable ALT+ENTER fullscreen switching.
  ThrowIfFailed(_device.GetDXGIFactory()->MakeWindowAssociation(
      hwnd, DXGI_MWA_NO_ALT_ENTER));
}

void SwapChain::CreateBackBufferView() {
  // Get the back buffer texture
  ThrowIfFailed(_swapchain->GetBuffer(0, IID_PPV_ARGS(&_backBuffer)));

  // Create a render target view of the back buffer texture
  _device.GetDevice()->CreateRenderTargetView(_backBuffer.Get(), nullptr,
                                              &_backBufferRTV);

  // Get the description of the backbuffer image
  _backBuffer->GetDesc(&_backbufferDesc);

  // Get the width and height
  _width = _backbufferDesc.Width;
  _height = _backbufferDesc.Height;
}
