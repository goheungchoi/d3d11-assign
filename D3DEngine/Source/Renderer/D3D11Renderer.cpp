#include "D3DEngine/Renderer/D3D11Renderer.h"

#include "Internal/Resources/RenderDevice.h"
#include "Internal/Resources/SwapChain.h"
using namespace DX;

#include "directxtk/DDSTextureLoader.h"
using namespace DirectX;

HRESULT D3D11Renderer::Initialize(HWND hWnd, UINT width, UINT height) {

	_device = RenderDevice::CreateRenderDevice(true, true);

  return E_NOTIMPL;
}

void D3D11Renderer::Shutdown() {}

void D3D11Renderer::BeginDraw() {}

void D3D11Renderer::EndDraw() {}
