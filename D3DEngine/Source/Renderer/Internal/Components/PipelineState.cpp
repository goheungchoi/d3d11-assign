#include "PipelineState.h"

void DX::PipelineStateBuilder::Reset() { 
	
	_stateAbstract = {};

	_inputLayoutDesc.clear();

	_topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

	_viewport = {};
  _scissor = {};

	_rasterizerDesc = {};
  _depthStencilDesc = {};

	_vs = Handle::kInvalidHandle;
  _ps = Handle::kInvalidHandle;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::IAInputTopology(
    D3D_PRIMITIVE_TOPOLOGY topology) {
  _topology = topology;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::IAInputLayout(
    IAInputLayoutFlags flags) {
  if ((flags & IAInputLayoutFlagBit::kNormal) != 0) {
  }

	if ((flags & IAInputLayoutFlagBit::kTangent) != 0) {
  }

  if ((flags & IAInputLayoutFlagBit::kBitangent) != 0) {
  }

  if ((flags & IAInputLayoutFlagBit::kTexcoord) != 0) {
  }

  if ((flags & IAInputLayoutFlagBit::kColor) != 0) {
  }
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::VSSetVertexShader(Handle vs) {
  _vs = vs;
  return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::RSSetViewport(UINT x, UINT y,
                                                              UINT width,
                                                              UINT height) {
  _viewport = {
      .TopLeftX = static_cast<float>(x),
      .TopLeftY = static_cast<float>(y),
      .Width = static_cast<float>(width),
      .Height = static_cast<float>(height),
      .MinDepth = 0.f,
      .MaxDepth = 1.f,
  };
  return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::RSSetFillMode(
    D3D11_FILL_MODE fillMode) {
  _rasterizerDesc.FillMode = fillMode;
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::RSSetCullMode(
    D3D11_CULL_MODE cullMode) {
  _rasterizerDesc.CullMode = cullMode;
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::RSEnableFrontFaceCounterClockwise() {
  _rasterizerDesc.FrontCounterClockwise = TRUE;
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::RSEnableDepthClipping() {
  _rasterizerDesc.DepthClipEnable = TRUE;
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::RSEnableScissor(
    D3D11_RECT scissor) {
  _rasterizerDesc.ScissorEnable = TRUE;
  _scissor = scissor;
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::RSDisableMultisample() {
  _rasterizerDesc.MultisampleEnable = FALSE;
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::RSEnableMultisample(
    UINT samples) {
  _rasterizerDesc.MultisampleEnable = TRUE;
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::PSSetPixelShader(Handle ps) {
  _ps = ps;
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::OMDisableDepthTesting() {
  _depthStencilDesc.DepthEnable = FALSE;
  _depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::OMEnableDepthTesting(
    D3D11_COMPARISON_FUNC compOp) {
  _depthStencilDesc.DepthEnable = TRUE;
  _depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::OMDisableBlending() {
  _blendDesc.
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::OMEnableAdditiveBlending() {
  // TODO: insert return statement here
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::OMEnableAlphaBlending() {
  // TODO: insert return statement here
	return *this;
}

DX::PipelineState DX::PipelineStateBuilder::Build() { return PipelineState(); }
