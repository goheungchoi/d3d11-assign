#pragma once

#include "Renderer/Internal/D3D11Common.h"

namespace DX {

DXGI_SWAP_CHAIN_DESC1 CreateDXGISwapChainDesc1(UINT width, UINT height) {
  DXGI_SWAP_CHAIN_DESC1 desc1{
      .Width = width,
      .Height = height,
      // Swap buffer description
      .Format = DXGI_FORMAT_R8G8B8A8_UNORM,
      // Sample description
      .SampleDesc =
          {
              .Count = 1,  // No multisampling
              .Quality = 0,
          },
      // Swap buffer usage
      .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
      // The number of buffers to use
      .BufferCount = 2,
      .Scaling = DXGI_SCALING_NONE,
      // Swap effect
      .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
      // Alpha mode
      .AlphaMode = DXGI_ALPHA_MODE_IGNORE,

      // TODO: Allow full screen switching
			// TODO: Allow tearing for v-sync off
      //.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
  };

	return desc1;
}


D3D11_TEXTURE2D_DESC Texture2DDesc(DXGI_FORMAT format, UINT width, UINT height,
                                   D3D11_BIND_FLAG binds) {
	

}

D3D11_TEXTURE2D_DESC Texture2DDesc(DXGI_FORMAT format, UINT width, UINT height,
                                 UINT mipLevel, D3D11_BIND_FLAG binds,
                                 D3D11_USAGE usage, D3D11_CPU_ACCESS_FLAG access);

}


