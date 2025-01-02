#pragma once

#include "Renderer/Internal/D3D11Common.h"

//struct FrameBuffer {
//  DXGI_FORMAT colorFormat, depthFormat;
//  ComPtr<ID3D11Texture2D> colorTexture;
//  ComPtr<ID3D11Texture2D> depthStencilTexture;
//  ComPtr<ID3D11RenderTargetView> rtv;
//  ComPtr<ID3D11ShaderResourceView> colorSRV;
//  ComPtr<ID3D11DepthStencilView> dsv;
//  ComPtr<ID3D11ShaderResourceView> depthSRV;
//  UINT width, height;
//  UINT samples;
//};

struct AttachmentInfo {
  DXGI_FORMAT format;
  UINT samples;
};

// Framebuffers represent a collection of memory attachments that are used by a
// render pass instance.
class FrameBuffer {
	struct Attachment {
    ComPtr<ID3D11View> view;
    D3D11_BIND_FLAG bind;
    DXGI_FORMAT format;
    UINT samples;
	};
  std::unordered_map<UINT, Attachment> _colorAttachments;
  Attachment _depthStencilAttachment;

public:

	void SetColorAttachment(UINT slot, ComPtr<ID3D11RenderTargetView>& rtv,
		const AttachmentInfo& info) {
   _colorAttachments[slot] =
       Attachment{rtv, D3D11_BIND_RENDER_TARGET, info.format, info.samples};
	}

	void SetDepthStencilAttachment(ComPtr<ID3D11DepthStencilView>& dsv,
		const AttachmentInfo& info) {
    _depthStencilAttachment =
        Attachment{dsv, D3D11_BIND_DEPTH_STENCIL, info.format, info.samples};
	}

	std::vector<std::pair<UINT, ComPtr<ID3D11RenderTargetView>>> GetAllColorAttachments() {
    std::vector<std::pair<UINT, ComPtr<ID3D11RenderTargetView>>> v;
		for (auto& [slot, attachment] : _colorAttachments) {
      ComPtr<ID3D11RenderTargetView> view;
      ThrowIfFailed(attachment.view.As(&view));
      v.emplace_back(std::pair{slot, view});
		}
    return v;
	}

	ComPtr<ID3D11RenderTargetView> GetColorAttachment(UINT slot) {
    auto it = _colorAttachments.find(slot);
    if (it == _colorAttachments.end())
      return {};
    else {
      ComPtr<ID3D11RenderTargetView> view;
      ThrowIfFailed(it->second.view.As(&view));
      return view;
		}
	}

	ComPtr<ID3D11DepthStencilView> GetDepthStencilAttachment() {
    ComPtr<ID3D11DepthStencilView> view;
    ThrowIfFailed(_depthStencilAttachment.view.As(&view));
    return view;
  }

};
