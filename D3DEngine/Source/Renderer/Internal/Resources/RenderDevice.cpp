#include "RenderDevice.h"

#include "RenderContext.h"
#include "SwapChain.h"

#include "nvpro/nv_dds.h"
#include <directxtk/DDSTextureLoader.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")

namespace {
// Utility depth format mapper to choose appropriate formats
// for different kinds of depth views.
struct DepthFormatMapping {
  DXGI_FORMAT typelessFormat;
  DXGI_FORMAT dsvFormat;
  DXGI_FORMAT srvFormat;
};

DepthFormatMapping GetDepthFormatMapping(DXGI_FORMAT requestedFormat) {
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

}

ComPtr<ID3D11Buffer> DX::RenderDevice::CreateConstantBuffer(D3D11_USAGE usage,
                                                            const void* data,
                                                            UINT size) const {
  D3D11_BUFFER_DESC desc = {};
  desc.ByteWidth = static_cast<UINT>(size);
  desc.Usage = usage;
  desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  desc.CPUAccessFlags =
      usage == D3D11_USAGE_DYNAMIC ? D3D11_CPU_ACCESS_WRITE : 0;

  D3D11_SUBRESOURCE_DATA bufferData = {};
  bufferData.pSysMem = data;

  ComPtr<ID3D11Buffer> buffer;
  const D3D11_SUBRESOURCE_DATA* bufferDataPtr = data ? &bufferData : nullptr;
  if (FAILED(_d3dDevice->CreateBuffer(&desc, bufferDataPtr, &buffer))) {
    throw std::runtime_error("Failed to create constant buffer");
  }
  return buffer;
}

void DX::RenderDevice::CopyMappedData(ComPtr<ID3D11Buffer>& buffer,
                                      const void* data, UINT size) {
   D3D11_MAPPED_SUBRESOURCE mappedSubresource;
  _d3dImmContext->Map(buffer.Get(), NULL, D3D11_MAP_WRITE_DISCARD, NULL,
                       &mappedSubresource);
   memcpy(mappedSubresource.pData, data, size);
  _d3dImmContext->Unmap(buffer.Get(), NULL);
}

void DX::RenderDevice::CopyData(ComPtr<ID3D11Buffer>& buffer,
                                const void* data) {
  _d3dImmContext->UpdateSubresource(buffer.Get(), 0, nullptr, data, 0, 0);
}

DX::MeshBuffer DX::RenderDevice::CreateMeshBuffer(const MeshData& data) {
  MeshBuffer mesh;

	// Create a vertex buffer.
	D3D11_BUFFER_DESC vertexBufferDesc{
      .ByteWidth = (UINT)(sizeof(Vertex) * std::size(data.vertices)),
      .Usage = D3D11_USAGE_IMMUTABLE,
      .BindFlags = D3D11_BIND_VERTEX_BUFFER,
      .CPUAccessFlags = 0};

	// Vertex buffer data
	D3D11_SUBRESOURCE_DATA vertexBufferSubresource{.pSysMem =
                                                     data.vertices.data()};

	if (FAILED(_d3dDevice->CreateBuffer(&vertexBufferDesc,
                                      &vertexBufferSubresource,
                                      mesh.vertexBuffer.GetAddressOf()))) {
    throw std::runtime_error("Failed to create a vertex buffer.");
	}

	mesh.stride = sizeof(Vertex);
  mesh.offset = 0U;
  mesh.numElements = (UINT)std::size(data.vertices);

	// Create an index buffer.
  D3D11_BUFFER_DESC indexBufferDesc{
      .ByteWidth = (UINT)(sizeof(Index) * std::size(data.indices)),
      .Usage = D3D11_USAGE_IMMUTABLE,
      .BindFlags = D3D11_BIND_INDEX_BUFFER};

	// Index buffer data
	D3D11_SUBRESOURCE_DATA indexBufferSubresource{.pSysMem = data.indices.data()};

  if (FAILED(_d3dDevice->CreateBuffer(&indexBufferDesc, &indexBufferSubresource,
                                      mesh.indexBuffer.GetAddressOf()))) {
    throw std::runtime_error("Failed to create an index buffer.");
  }

	return mesh;
}

DX::TextureBuffer DX::RenderDevice::CreateTextureBuffer(UINT width, UINT height,
                                                    DXGI_FORMAT format,
                                                    D3D11_BIND_FLAG flags,
                                                    UINT mipLevels) {
  DX::TextureBuffer texture;
  texture.format = format;
  texture.width = width;
  texture.height = height;
  texture.levels = mipLevels;

  D3D11_TEXTURE2D_DESC desc{};
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = mipLevels;
  desc.ArraySize = 1;
  desc.Format = format;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | flags;
  desc.CPUAccessFlags = 0;

  // Create a texture
  if (FAILED(_d3dDevice->CreateTexture2D(&desc, nullptr, &texture.texture))) {
    throw std::runtime_error("Failed to create Texture color texture.");
  }

	// Create shader resource view
  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
  srvDesc.Format = format;
  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MostDetailedMip = 0;
  srvDesc.Texture2D.MipLevels = mipLevels;
  if (FAILED(_d3dDevice->CreateShaderResourceView(texture.texture.Get(),
                                                  &srvDesc, &texture.srv))) {
    throw std::runtime_error(
        "Failed to create Texture shader resource view");
  }

	if ((flags & D3D11_BIND_UNORDERED_ACCESS) != 0) {
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
    uavDesc.Format = format;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = mipLevels;
    if (FAILED(_d3dDevice->CreateUnorderedAccessView(texture.texture.Get(),
                                                    &uavDesc, &texture.uav))) {
      throw std::runtime_error(
          "Failed to create Texture unordered access view");
    }
	}

  return texture;
}

DX::TextureBuffer DX::RenderDevice::CreateTextureBuffer(
    void* data, UINT pixelByteSize, UINT width, UINT height, DXGI_FORMAT format,
    D3D11_BIND_FLAG flags, UINT mipLevels) {
  DX::TextureBuffer texture;
  texture.format = format;
  texture.width = width;
  texture.height = height;
  texture.levels = mipLevels;

  D3D11_TEXTURE2D_DESC desc{};
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = mipLevels;
  desc.ArraySize = 1;
  desc.Format = format;
  desc.SampleDesc.Count = 1;
  desc.Usage = D3D11_USAGE_IMMUTABLE;
  desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | flags;
  desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA subresource{.pSysMem = data,
                                     .SysMemPitch = pixelByteSize * width};

  // Create a texture
  if (FAILED(
          _d3dDevice->CreateTexture2D(&desc, &subresource, &texture.texture))) {
    throw std::runtime_error("Failed to create Texture color texture.");
  }

  // Create shader resource view
  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
  srvDesc.Format = format;
  srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MostDetailedMip = 0;
  srvDesc.Texture2D.MipLevels = mipLevels;
  if (FAILED(_d3dDevice->CreateShaderResourceView(texture.texture.Get(),
                                                  &srvDesc, &texture.srv))) {
    throw std::runtime_error("Failed to create Texture shader resource view");
  }

  if ((flags & D3D11_BIND_UNORDERED_ACCESS) != 0) {
    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
    uavDesc.Format = format;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = mipLevels;
    if (FAILED(_d3dDevice->CreateUnorderedAccessView(texture.texture.Get(),
                                                     &uavDesc, &texture.uav))) {
      throw std::runtime_error(
          "Failed to create Texture unordered access view");
    }
  }

  return texture;
}

DX::TextureBuffer DX::RenderDevice::CreateTextureBuffer(const TextureData& data,
                                                        D3D11_BIND_FLAG flags) {
  // Create the texture buffer
  DX::TextureBuffer texture =
      CreateTextureBuffer(data.width, data.height, data.format, flags, data.mipLevels);

	nv_dds::Image image;
  nv_dds::ErrorWithText maybeError = image.readFromMemory(
      (const char*)data.ddsData.data(), data.ddsData.size(), {});
  if (maybeError.has_value()) {
    // Do something with the error message, maybeError.value()
    throw std::runtime_error("Failed to create a texture: " + maybeError.value());
  } else {
    // Access subresources using image.subresource(...), and upload them
    // to the GPU using your graphics API of choice.

		const UINT mipLevels = image.getNumMips();
    for (UINT i = 0; i < mipLevels; ++i) {
      nv_dds::Subresource& subresource = image.subresource(/* mip */ i);

      UINT mipHeight = image.getHeight(i);
			UINT mipSize = subresource.data.size();

			// Calculate row pitch
      UINT rowPitch = mipSize / mipHeight;
			
      _d3dImmContext->UpdateSubresource(texture.texture.Get(),
                                        D3D11CalcSubresource(i, 0, mipLevels),
                                        nullptr, subresource.data.data(),
                                        rowPitch, 0);
		}
  }

  return texture;
}

DX::CubeTextureBuffer DX::RenderDevice::CreateCubeTextureBuffer(
    UINT width, UINT height, DXGI_FORMAT format, D3D11_BIND_FLAG flags,
    UINT mipLevels, UINT arrayLayers) {
  // TODO:
	return CubeTextureBuffer();
}

DX::RenderTargetBuffer DX::RenderDevice::CreateRenderTargetBuffer(
    UINT width, UINT height, DXGI_FORMAT format, UINT samples) {
	// TODO:
  // Get the maximun sample count of the formats
  UINT colorSampleCountMax = GetMultisampleMaxCount(format);
  if (samples > colorSampleCountMax) {
    std::cout << "The max sample count of render target in type (" << format
              << ") is " << colorSampleCountMax << ", but " << samples
              << " was inputted." << std::endl;
  }

	RenderTargetBuffer renderTarget;
  renderTarget.format = format;
  renderTarget.width = width;
  renderTarget.height = height;
  renderTarget.samples = samples;

	D3D11_TEXTURE2D_DESC desc{};
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.SampleDesc.Count = samples;

	// Create a color texture
  desc.Format = format;
  desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
  if (FAILED(_d3dDevice->CreateTexture2D(&desc, nullptr, &renderTarget.texture))) {
    throw std::runtime_error("Failed to create RenderTarget color texture.");
  }

  // Create render target view
  D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
  rtvDesc.Format = format;
  rtvDesc.ViewDimension = (samples > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMS
                                        : D3D11_RTV_DIMENSION_TEXTURE2D;
  if (FAILED(_d3dDevice->CreateRenderTargetView(renderTarget.texture.Get(),
                                                &rtvDesc, &renderTarget.rtv))) {
    throw std::runtime_error(
        "Failed to create RenderTarget render target view.");
  }

  // Create shader resource view
  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
  srvDesc.Format = format;
  srvDesc.ViewDimension = (samples > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMS
                                        : D3D11_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MostDetailedMip = 0;
  srvDesc.Texture2D.MipLevels = 1;
  if (FAILED(_d3dDevice->CreateShaderResourceView(renderTarget.texture.Get(),
                                                  &srvDesc, &renderTarget.srv))) {
    throw std::runtime_error(
        "Failed to create RenderTarget shader resource view");
  }

  return renderTarget;
}

DX::DepthStensilBuffer DX::RenderDevice::CreateDepthStencilBuffer(
    UINT width, UINT height, DXGI_FORMAT format, UINT samples) {
	
	UINT depthSampleCountMax = GetMultisampleMaxCount(format);
  if (samples > depthSampleCountMax) {
    std::cout << "The max sample count of frame buffer in type (" << format
              << ") is " << depthSampleCountMax << ", but " << samples
              << " was inputted." << std::endl;
  }

	DepthStensilBuffer depthBuffer;
  depthBuffer.format = format;
  depthBuffer.width = width;
  depthBuffer.height = height;
  depthBuffer.samples = samples;

	D3D11_TEXTURE2D_DESC desc{};
  desc.Width = width;
  desc.Height = height;
  desc.MipLevels = 1;
  desc.ArraySize = 1;
  desc.SampleDesc.Count = samples;

	// Get the depth mapping
  auto depthMapping = GetDepthFormatMapping(format);

  // Create depth stencil texture
  desc.Format = depthMapping.typelessFormat;
  desc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
  if (FAILED(_d3dDevice->CreateTexture2D(&desc, nullptr,
                                         &depthBuffer.texture))) {
    throw std::runtime_error(
        "Failed to create DepthBuffer depth-stencil texture");
  }

  // Create depth stencil view
  D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
  dsvDesc.Format = depthMapping.dsvFormat;
  dsvDesc.ViewDimension = (samples > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS
                                        : D3D11_DSV_DIMENSION_TEXTURE2D;
  if (FAILED(_d3dDevice->CreateDepthStencilView(depthBuffer.texture.Get(),
                                                &dsvDesc, &depthBuffer.dsv))) {
    throw std::runtime_error("Failed to create DepthBuffer depth-stencil view");
  }

  // Create shader resource view
  D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
  srvDesc.Format = depthMapping.srvFormat;
  srvDesc.ViewDimension = (samples > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMS
                                        : D3D11_SRV_DIMENSION_TEXTURE2D;
  srvDesc.Texture2D.MostDetailedMip = 0;
  srvDesc.Texture2D.MipLevels = 1;
  if (FAILED(_d3dDevice->CreateShaderResourceView(
          depthBuffer.texture.Get(), &srvDesc, &depthBuffer.srv))) {
    throw std::runtime_error(
        "Failed to create DepthBuffer shader resource view");
  }

  return depthBuffer;
}

DX::FrameBuffer* DX::RenderDevice::CreateFrameBuffer(
    UINT width, UINT height, UINT samples,
    std::initializer_list<RenderTargetBuffer> colorAttachments,
    std::optional<DepthStensilBuffer> depthAttachment) {
	// Set width and height of frame buffer.
  DX::FrameBuffer* frameBuf = new DX::FrameBuffer{width, height, samples};

	// Attach render targets
	for (int i = 0; i < colorAttachments.size(); ++i) {
    auto it = colorAttachments.begin() + i;

		AttachmentInfo attachment{.view = it->rtv,
                              .bind = D3D11_BIND_RENDER_TARGET,
                              .format = it->format,
                              .width = it->width,
                              .height = it->height,
                              .samples = it->samples};

    frameBuf->SetColorAttachment(i, attachment);
	}

	// Attach depth buffer
	if (depthAttachment) {
    AttachmentInfo attachment{.view = depthAttachment->dsv,
                              .bind = D3D11_BIND_DEPTH_STENCIL,
                              .format = depthAttachment->format,
                              .width = depthAttachment->width,
                              .height = depthAttachment->height,
                              .samples = depthAttachment->samples};
		
		frameBuf->SetDepthStencilAttachment(attachment);
	}

  return frameBuf;
}

ComPtr<ID3D11SamplerState> DX::RenderDevice::CreateSamplerState(
    D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode) const {
  D3D11_SAMPLER_DESC desc = {};
  desc.Filter = filter;
  desc.AddressU = addressMode;
  desc.AddressV = addressMode;
  desc.AddressW = addressMode;
  desc.MaxAnisotropy =
      (filter == D3D11_FILTER_ANISOTROPIC) ? D3D11_REQ_MAXANISOTROPY : 1;
  desc.MinLOD = 0;
  desc.MaxLOD = D3D11_FLOAT32_MAX;

  ComPtr<ID3D11SamplerState> samplerState;
  if (FAILED(_d3dDevice->CreateSamplerState(&desc, &samplerState))) {
    throw std::runtime_error("Failed to create sampler state");
  }
  return samplerState;
}

//DX::FrameBuffer DX::RenderDevice::CreateFrameBuffer(UINT width, UINT height,
//                                                DXGI_FORMAT colorFormat,
//                                                DXGI_FORMAT depthFormat,
//                                                UINT sampleCount) {
//	// Get the maximun sample count of the formats
//  UINT colorSampleCountMax = UINT_MAX;
//	if (colorFormat != DXGI_FORMAT_UNKNOWN) {
//		colorSampleCountMax = GetMultisampleMaxCount(colorFormat);
//		if (sampleCount > colorSampleCountMax) {
//			std::cout << "The max sample count of frame buffer in type (" << colorFormat
//								<< ") is " << colorSampleCountMax << ", but " << sampleCount
//								<< " was inputted." << std::endl;
//		}
//	}
//	
//  UINT depthSampleCountMax = UINT_MAX;
//	if (depthFormat != DXGI_FORMAT_UNKNOWN) {
//    depthSampleCountMax = GetMultisampleMaxCount(depthFormat);
//		if (sampleCount > depthSampleCountMax) {
//			std::cout << "The max sample count of frame buffer in type (" << depthFormat
//								<< ") is " << depthSampleCountMax << ", but " << sampleCount
//								<< " was inputted." << std::endl;
//		}
//	}
//
//	sampleCount = std::min({sampleCount, colorSampleCountMax, depthSampleCountMax});
//
//	// Get the quality levels
//  UINT colorQualityLevels =
//      (colorFormat != DXGI_FORMAT_UNKNOWN)
//          ? GetMultisampleQualityLevels(colorFormat, sampleCount)
//          : 0;
//  UINT depthQualityLevels =
//      (depthFormat != DXGI_FORMAT_UNKNOWN)
//          ? GetMultisampleQualityLevels(depthFormat, sampleCount)
//          : 0;
//
//
//	// Create the frame buffer
//  FrameBuffer fb;
//  fb.colorFormat = colorFormat;
//  fb.depthFormat = depthFormat;
//  fb.width = width;
//  fb.height = height;
//  fb.samples = sampleCount;
//
//	D3D11_TEXTURE2D_DESC desc{};
//  desc.Width = width;
//  desc.Height = height;
//  desc.MipLevels = 1;
//  desc.ArraySize = 1;
//  desc.SampleDesc.Count = sampleCount;
//
//	// Create color attechment
//  if (colorFormat != DXGI_FORMAT_UNKNOWN) {
//    // Create a color texture
//    desc.Format = colorFormat;
//    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
//    if (FAILED(_d3dDevice->CreateTexture2D(&desc, nullptr, &fb.colorTexture))) {
//      throw std::runtime_error("Failed to create FrameBuffer color texture.");
//    }
//
//    // Create render target view
//    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc{};
//    rtvDesc.Format = colorFormat;
//    rtvDesc.ViewDimension = (sampleCount > 1) ? D3D11_RTV_DIMENSION_TEXTURE2DMS
//                                              : D3D11_RTV_DIMENSION_TEXTURE2D;
//    if (FAILED(_d3dDevice->CreateRenderTargetView(fb.colorTexture.Get(),
//                                                  &rtvDesc, &fb.rtv))) {
//      throw std::runtime_error(
//          "Failed to create FrameBuffer render target view.");
//    }
//
//    // Create shader resource view
//    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
//    srvDesc.Format = colorFormat;
//    srvDesc.ViewDimension = (sampleCount > 1) ? D3D11_SRV_DIMENSION_TEXTURE2DMS
//                                              : D3D11_SRV_DIMENSION_TEXTURE2D;
//    srvDesc.Texture2D.MostDetailedMip = 0;
//    srvDesc.Texture2D.MipLevels = 1;
//    if (FAILED(_d3dDevice->CreateShaderResourceView(fb.colorTexture.Get(),
//                                                    &srvDesc, &fb.colorSRV))) {
//      throw std::runtime_error(
//          "Failed to create FrameBuffer shader resource view");
//    }
//  }
//
//
//	
//
//	// Create depth-stencil attachment
//  if (depthFormat != DXGI_FORMAT_UNKNOWN) {
//
//  }
//
//  return fb;
//}

DX::SwapChain* DX::RenderDevice::CreateSwapChain(HWND hwnd, UINT width,
                                             UINT height, bool allowTearing) {
  DX::SwapChain* swapchain = new DX::SwapChain(*this);
  swapchain->Initialize(hwnd, width, height, allowTearing);
	return swapchain;
}

DX::RenderContext* DX::RenderDevice::CreateRenderContext() { 
	RenderContext* renderContext = new RenderContext(this);
  _d3dDevice->CreateDeferredContext(
      0, renderContext->_deferredContext.GetAddressOf());
  return renderContext;
}

void DX::RenderDevice::SubmitCommandList(RenderContext* renderContext) {
  _d3dImmContext->ExecuteCommandList(renderContext->_commandList.Get(), FALSE);
}
