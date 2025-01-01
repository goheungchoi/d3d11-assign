#pragma once

#include "PipelineState.h"

#include "FrameBuffer.h"

class RenderPass {

	PipelineState _state;

	// Frame Buffers
  FrameBuffer _fbo;

public:

	// TODO:

	void SetPipelineState(PipelineState state);

	void BindConstantBuffer(Handle buffer, UINT slot);
  void BindBuffer(Handle buffer, UINT slot);
	void BindTexture(Handle texture, UINT slot);
  void BindSamplers(Handle sampler, UINT slot);

	void BindFrameBuffer(FrameBuffer frameBuffers);

	void Execute(class DX::RenderContext* context);

};

