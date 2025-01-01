#include "RenderDevice.h"

#include "RenderContext.h"
#include "SwapChain.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

TextureBuffer DX::RenderDevice::CreateTextureBuffer(UINT width, UINT height,
                                                    DXGI_FORMAT format,
                                                    D3D11_BIND_FLAG flags,
                                                    UINT mipLevels) {
	// TODO:
  return TextureBuffer();
}

TextureBuffer DX::RenderDevice::CreateTextureBuffer(void* data, UINT width,
                                                    UINT height,
                                                    DXGI_FORMAT format,
                                                    D3D11_BIND_FLAG flags,
                                                    UINT mipLevels) {
  return TextureBuffer();
}

CubeTextureBuffer DX::RenderDevice::CreateCubeTextureBuffer(
    UINT width, UINT height, DXGI_FORMAT format, D3D11_BIND_FLAG flags,
    UINT mipLevels, UINT arrayLayers) {
  // TODO:
	return CubeTextureBuffer();
}

FrameBuffer DX::RenderDevice::CreateFrameBuffer(UINT width, UINT height,
                                                DXGI_FORMAT colorFormat,
                                                DXGI_FORMAT depthFormat,
                                                UINT sampleCount) {
	// Get the maximun sample count of the formats
  UINT colorSampleCountMax = UINT_MAX;
	if (colorFormat != DXGI_FORMAT_UNKNOWN) {
		colorSampleCountMax = GetMultisampleMaxCount(colorFormat);
		if (sampleCount > colorSampleCountMax) {
			std::cout << "The max sample count of frame buffer in type (" << colorFormat
								<< ") is " << colorSampleCountMax << ", but " << sampleCount
								<< " was inputted." << std::endl;
		}
	}

  UINT depthSampleCountMax = UINT_MAX;
	if (depthFormat != DXGI_FORMAT_UNKNOWN) {
    depthSampleCountMax = GetMultisampleMaxCount(depthFormat);
		if (sampleCount > depthSampleCountMax) {
			std::cout << "The max sample count of frame buffer in type (" << depthFormat
								<< ") is " << depthSampleCountMax << ", but " << sampleCount
								<< " was inputted." << std::endl;
		}
	}

	sampleCount = std::min({sampleCount, colorSampleCountMax, depthSampleCountMax});

	// Get the quality levels
  UINT colorQualityLevels =
      (colorFormat != DXGI_FORMAT_UNKNOWN)
          ? GetMultisampleQualityLevels(colorFormat, sampleCount)
          : 0;
  UINT depthQualityLevels =
      (depthFormat != DXGI_FORMAT_UNKNOWN)
          ? GetMultisampleQualityLevels(depthFormat, sampleCount)
          : 0;


	// Create the frame buffer
  FrameBuffer fb;
  fb.colorFormat = colorFormat;
  fb.depthFormat = depthFormat;
  fb.width = width;
  fb.height = height;
  fb.samples = sampleCount;

	D3D11_TEXTURE2D_DESC desc{};
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.SampleDesc.Count = sampleCount;

	// Create color attechment
  if (colorFormat != DXGI_FORMAT_UNKNOWN) {
    // Create a color texture
    desc.Format = colorFormat;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    if (FAILED(_d3dDevice->CreateTexture2D(&desc, nullptr, &fb.colorTexture))) {
      throw std::runtime_error("Failed to create FrameBuffer color texture.");
    }

    // Create render target view
    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
    rtvDesc.Format = colorFormat;
    rtvDesc.ViewDimension = (sampleCount > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMS
                                              : D3D11_RTV_DIMENSION_TEXTURE2D;
    if (FAILED(_d3dDevice->CreateRenderTargetView(fb.colorTexture.Get(),
                                                  &rtvDesc, &fb.rtv))) {
      throw std::runtime_error(
          "Failed to create FrameBuffer render target view.");
    }

    // Create shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = colorFormat;
    srvDesc.ViewDimension = (sampleCount > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMS
                                              : D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    if (FAILED(_d3dDevice->CreateShaderResourceView(fb.colorTexture.Get(),
                                                    &srvDesc, &fb.colorSRV))) {
      throw std::runtime_error(
          "Failed to create FrameBuffer shader resource view");
    }
  }


	// Utility depth format mapper to choose appropriate formats
	// for different kinds of depth views.
  struct DepthFormatMapping {
    DXGI_FORMAT typelessFormat;
    DXGI_FORMAT dsvFormat;
    DXGI_FORMAT srvFormat;
  };

  auto GetDepthFormatMapping =
      [](DXGI_FORMAT requestedFormat) -> DepthFormatMapping {
    switch (requestedFormat) {
      case DXGI_FORMAT_D24_UNORM_S8_UINT:
        return {DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT,
                DXGI_FORMAT_R24_UNORM_X8_TYPELESS};
      case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
        return {DXGI_FORMAT_R32G8X24_TYPELESS, DXGI_FORMAT_D32_FLOAT_S8X24_UINT,
                DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS};
      case DXGI_FORMAT_D32_FLOAT:
        return {DXGI_FORMAT_R32_TYPELESS, DXGI_FORMAT_D32_FLOAT,
                DXGI_FORMAT_R32_FLOAT};
      case DXGI_FORMAT_D16_UNORM:
        return {DXGI_FORMAT_R16_TYPELESS, DXGI_FORMAT_D16_UNORM,
                DXGI_FORMAT_R16_UNORM};
      default:
        throw std::invalid_argument("Unsupported depth format requested.");
    }
  };

	// Create depth-stencil attachment
  if (depthFormat != DXGI_FORMAT_UNKNOWN) {
		// Get the depth mapping
    auto depthMapping = GetDepthFormatMapping(depthFormat);

		// Create depth stencil texture
    desc.Format = depthMapping.typelessFormat;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    if (FAILED(_d3dDevice->CreateTexture2D(&desc, nullptr,
                                            &fb.depthStencilTexture))) {
      throw std::runtime_error(
          "Failed to create FrameBuffer depth-stencil texture");
    }

		// Create depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
    dsvDesc.Format = depthMapping.dsvFormat;
    dsvDesc.ViewDimension = (sampleCount > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS
																							: D3D11_DSV_DIMENSION_TEXTURE2D;
    if (FAILED(_d3dDevice->CreateDepthStencilView(
            fb.depthStencilTexture.Get(), &dsvDesc, &fb.dsv))) {
      throw std::runtime_error(
          "Failed to create FrameBuffer depth-stencil view");
    }

		// Create shader resource view
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = depthMapping.srvFormat;
    srvDesc.ViewDimension = (sampleCount > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMS
                                              : D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;
    if (FAILED(_d3dDevice->CreateShaderResourceView(
            fb.depthStencilTexture.Get(), &srvDesc, &fb.depthSRV))) {
      throw std::runtime_error(
          "Failed to create FrameBuffer shader resource view");
    }
  }

  return fb;
}

DX::SwapChain* DX::RenderDevice::CreateSwapChain(HWND hwnd, UINT width,
                                             UINT height, bool allowTearing) {
  DX::SwapChain* swapchain = new DX::SwapChain(*this);
  swapchain->Initialize(hwnd, width, height, allowTearing);
	return swapchain;
}

DX::RenderContext* DX::RenderDevice::CreateRenderContext() { 
	RenderContext* renderContext = new RenderContext(*this);
  _d3dDevice->CreateDeferredContext(
      0, renderContext->_deferredContext.GetAddressOf());
  return renderContext;
}

void DX::RenderDevice::SubmitCommandList(RenderContext* renderContext) {
  _d3dImmContext->ExecuteCommandList(renderContext->_commandList.Get(), FALSE);

}
