#pragma once

#include "Renderer/Internal/D3D11Common.h"

namespace DX {

struct AttachmentInfo {
  ComPtr<ID3D11View> view;
  D3D11_BIND_FLAG bind;
  DXGI_FORMAT format;
  UINT width, height, samples;
};

// Framebuffers represent a collection of memory attachments that are used by a
// render pass instance.
class FrameBuffer {
  UINT _width{0}, _height{0}, _samples{0};
  std::unordered_map<UINT, AttachmentInfo> _colorAttachments;
  std::optional<AttachmentInfo> _depthStencilAttachment;

 public:

	FrameBuffer(UINT width, UINT height, UINT samples)
      : _width{width}, _height{height}, _samples{samples} {}

  void SetColorAttachment(UINT slot, const AttachmentInfo& attachment) {
		// Check the bind
    if ((attachment.bind & D3D11_BIND_RENDER_TARGET) == 0)
      throw std::exception("Color attachment bind must be render target");

		// Check if the view is actually RTV
		ComPtr<ID3D11RenderTargetView> rtv;
    ThrowIfFailed(attachment.view.As(&rtv));

		// Check the width, height, and samples if it matches with the frame
		if (attachment.width != _width || attachment.height != _height ||
        attachment.samples != _samples) {
      throw std::exception("Color attachment extent mismatch");
		}

    _colorAttachments[slot] = attachment;
  }

  void SetDepthStencilAttachment(const AttachmentInfo& attachment) {
    // Check the bind
    if ((attachment.bind & D3D11_BIND_DEPTH_STENCIL) == 0)
      throw std::exception("Color attachment bind must be render target");

    // Check if the view is actually DSV
    ComPtr<ID3D11DepthStencilView> dsv;
    ThrowIfFailed(attachment.view.As(&dsv));

		// Check the width, height, and samples if it matches with the frame
    if (attachment.width != _width || attachment.height != _height ||
        attachment.samples != _samples) {
      throw std::exception("Color attachment extent mismatch");
    }

    _depthStencilAttachment = attachment;
  }

  /*std::vector<std::pair<UINT, ComPtr<ID3D11RenderTargetView>>>
  GetAllColorAttachments() {
    std::vector<std::pair<UINT, ComPtr<ID3D11RenderTargetView>>> v;
    for (auto& [slot, attachment] : _colorAttachments) {
      ComPtr<ID3D11RenderTargetView> view;
      ThrowIfFailed(attachment.view.As(&view));
      v.emplace_back(std::pair{slot, view});
    }
    return v;
  }*/

	UINT GetWidth() const { return _width; }
  UINT GetHeight() const { return _height; }
  UINT GetSampleCount() const { return _samples; }
	UINT GetColorAttachmentCount() const { return _colorAttachments.size(); }

  std::optional<AttachmentInfo> GetColorAttachment(UINT slot) const {
    auto it = _colorAttachments.find(slot);
    if (it == _colorAttachments.end())
      return std::nullopt;
    else {
      return it->second;
    }
  }

  std::optional<AttachmentInfo> GetDepthStencilAttachment() const {
    return _depthStencilAttachment;
  }
};

}  // namespace DX