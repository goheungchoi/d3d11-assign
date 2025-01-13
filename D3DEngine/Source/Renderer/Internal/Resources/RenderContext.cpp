#include "RenderContext.h"

#include "Renderer/Internal/Components/RenderPass.h"

void DX::RenderContext::BeginRendering(const RenderPass& pass) {
  std::vector<ID3D11RenderTargetView*> rtvs;
  for (int i = 0; i < pass._fbo->GetColorAttachmentCount(); ++i) {
    auto attachment = pass._fbo->GetColorAttachment(i);
    if (attachment) {
      ComPtr<ID3D11RenderTargetView> view;
      ThrowIfFailed(attachment->view.As(&view));
      rtvs.push_back(view.Get());
		}
	}

	ComPtr<ID3D11DepthStencilView> dsv;
	auto depthAttachment = pass._fbo->GetDepthStencilAttachment();
  if (depthAttachment) {
    ThrowIfFailed(depthAttachment->view.As(&dsv));
	}

  _deferredContext->OMSetRenderTargets(rtvs.size(), rtvs.data(), dsv.Get());
}

void DX::RenderContext::BindPipelineState(const PipelineState& pipeline) {}

void DX::RenderContext::DrawMesh(Handle mesh) {}

void DX::RenderContext::EndRendering() {}
