#pragma once

#include "PipelineState.h"
#include "FrameBuffer.h"

namespace DX {

class RenderPass {
  PipelineState _state;

	MeshBuffer _mesh;

	std::unordered_map<UINT, ComPtr<ID3D11Buffer>> _constantBuffers;
  std::unordered_map<UINT, ComPtr<ID3D11ShaderResourceView>> _shaderResources;
  std::unordered_map<UINT, ComPtr<ID3D11SamplerState>> _samplers;

  // Frame Buffer
  FrameBuffer _fbo;

 public:
  // TODO:

  void SetPipelineState(PipelineState state);

	void BindMeshBuffer(Handle mesh);

  void BindConstantBuffer(UINT slot, Handle buffer);
  void BindResource(UINT slot, Handle resource);
  void BindSamplers(UINT slot, Handle sampler);

  void BindFrameBuffer(FrameBuffer frameBuffers);

	void Execute(class RenderContext& context);
};

}  // namespace DX
