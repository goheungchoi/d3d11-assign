#include "RenderDevice.h"

#include "SwapChain.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

DX::SwapChain* DX::RenderDevice::CreateSwapChain(HWND hwnd, UINT width,
                                             UINT height, bool allowTearing) {
  DX::SwapChain* swapchain = new DX::SwapChain(*this);
  swapchain->Initialize(hwnd, width, height, allowTearing);
	return swapchain;
}
