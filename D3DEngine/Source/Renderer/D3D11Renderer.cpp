#include "D3DEngine/Renderer/D3D11Renderer.h"

#include "Internal/Resources/RenderDevice.h"
#include "Internal/Resources/SwapChain.h"
using namespace DX;

#include "directxtk/DDSTextureLoader.h"
using namespace DirectX;

void D3D11Renderer::Initialize(HWND hWnd, UINT width, UINT height, bool allowTearing) {

	_device = RenderDevice::CreateRenderDevice();
  _swapchain = _device->CreateSwapChain(hWnd, width, height, allowTearing);

  
}

void D3D11Renderer::Shutdown() {}

void D3D11Renderer::BeginFrame() { 


	
}

void D3D11Renderer::BeginDraw() {}

void D3D11Renderer::EndDraw() {}

void D3D11Renderer::EndFrame() {}
