#include "RenderContext.h"

void DX::RenderContext::BeginRendering(const RenderPass& pass) {
  _deferredContext->OMSetRenderTargets()

}

void DX::RenderContext::BindPipelineState(const PipelineState& pipeline) {}

void DX::RenderContext::DrawMesh(Handle mesh) {}

void DX::RenderContext::EndRendering() {}
