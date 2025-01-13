#include "PipelineState.h"

#include "Renderer/Internal/Resources/RenderDevice.h"

#include "D3DEngine/ResourceManager/ResourceManager.h"

bool operator==(const DX::PipelineStateAbstract& lhs,
                const DX::PipelineStateAbstract& rhs) {
  /*lhs.viewport == rhs.viewport;
  lhs.scissor == rhs.scissor;

  lhs.topology == rhs.topology;
  lhs.inputLayout == rhs.inputLayout;

  lhs.fill == rhs.fill;
  lhs.cull == rhs.cull;
  lhs.frontClockwise == rhs.frontClockwise;
  lhs.multisampleCount == rhs.multisampleCount;
  lhs.depthClipEnabled == rhs.depthClipEnabled;
  lhs.scissorEnabled == rhs.scissorEnabled;

  lhs.depthEnabled == rhs.depthEnabled;
  lhs.depthCompOp == rhs.depthCompOp;

  lhs.blendMode == rhs.blendMode;

  lhs.vertexShader == rhs.vertexShader;
  lhs.pixelShader == rhs.pixelShader;*/

  return memcmp(&lhs, &rhs, sizeof(DX::PipelineStateAbstract)) == 0;
}

bool operator!=(const DX::PipelineStateAbstract& lhs,
                const DX::PipelineStateAbstract& rhs) {
  return !(lhs == rhs);
}

void DX::PipelineStateBuilder::Reset() { 
	
	_stateAbstract = {};

	_inputLayoutDesc.clear();

	_topology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;

	_viewport = {};
  _scissor = {};

	_rasterizerDesc = {};
  _depthStencilDesc = {};
  _blendDesc = {};

	_colorAttachmentFormats.clear();
  _depthStencilAttachmentFormat.reset();

	_vs = Handle::kInvalidHandle;
  _ps = Handle::kInvalidHandle;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::IAInputTopology(
    D3D_PRIMITIVE_TOPOLOGY topology) {
	// Set the state abstract info
  if (topology == D3D_PRIMITIVE_TOPOLOGY_POINTLIST)
    _stateAbstract.topology = kPointList;
  else if (topology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
    _stateAbstract.topology = kTriangleList;
  else {
    assert(false, "Unsupported input topology.");
	}

  _topology = topology;

	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::IAInputLayout(
    IAInputLayoutFlags flags) {
  // Set the state abstract info
  _stateAbstract.inputLayout = flags;

	// Byte offset of input elements
  UINT accumatedByteOffset = 0;

	// Position
  _inputLayoutDesc.push_back(
      D3D11_INPUT_ELEMENT_DESC{.SemanticName = "POSITION",
                               .SemanticIndex = 0,
                               .Format = DXGI_FORMAT_R32G32B32_FLOAT,
                               .InputSlot = 0,
                               .AlignedByteOffset = accumatedByteOffset,
                               .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
                               .InstanceDataStepRate = 0});
	accumatedByteOffset += 12;

	// Normal
  if ((flags & IAInputLayoutFlagBit::kNormal) != 0) {
    _inputLayoutDesc.push_back(
        D3D11_INPUT_ELEMENT_DESC{.SemanticName = "NORMAL",
                                 .SemanticIndex = 0,
                                 .Format = DXGI_FORMAT_R32G32B32_FLOAT,
                                 .InputSlot = 0,
                                 .AlignedByteOffset = accumatedByteOffset,
                                 .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
                                 .InstanceDataStepRate = 0});
  }
  accumatedByteOffset += 12;

	// Tangent
  if ((flags & IAInputLayoutFlagBit::kTangent) != 0) {
    _inputLayoutDesc.push_back(
        D3D11_INPUT_ELEMENT_DESC{.SemanticName = "TANGENT",
                                 .SemanticIndex = 0,
                                 .Format = DXGI_FORMAT_R32G32B32_FLOAT,
                                 .InputSlot = 0,
                                 .AlignedByteOffset = accumatedByteOffset,
                                 .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
                                 .InstanceDataStepRate = 0});
  }
  accumatedByteOffset += 12;

	// Bitangent
  if ((flags & IAInputLayoutFlagBit::kBitangent) != 0) {
    _inputLayoutDesc.push_back(
        D3D11_INPUT_ELEMENT_DESC{.SemanticName = "BITANGENT",
                                 .SemanticIndex = 0,
                                 .Format = DXGI_FORMAT_R32G32B32_FLOAT,
                                 .InputSlot = 0,
                                 .AlignedByteOffset = accumatedByteOffset,
                                 .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
                                 .InstanceDataStepRate = 0});
  }
  accumatedByteOffset += 12;

	// Texcoord
  if ((flags & IAInputLayoutFlagBit::kTexcoord) != 0) {
    _inputLayoutDesc.push_back(
        D3D11_INPUT_ELEMENT_DESC{.SemanticName = "TEXCOORD",
                                 .SemanticIndex = 0,
                                 .Format = DXGI_FORMAT_R32G32_FLOAT,
                                 .InputSlot = 0,
                                 .AlignedByteOffset = accumatedByteOffset,
                                 .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
                                 .InstanceDataStepRate = 0});
  }
  accumatedByteOffset += 8;

	// Color
  if ((flags & IAInputLayoutFlagBit::kColor) != 0) {
    _inputLayoutDesc.push_back(
        D3D11_INPUT_ELEMENT_DESC{.SemanticName = "COLOR",
                                 .SemanticIndex = 0,
                                 .Format = DXGI_FORMAT_R32G32B32_FLOAT,
                                 .InputSlot = 0,
                                 .AlignedByteOffset = accumatedByteOffset,
                                 .InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
                                 .InstanceDataStepRate = 0});
  }
  accumatedByteOffset += 12;

	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::VSSetVertexShader(Handle vs) {
  // Set the state abstract info
  _stateAbstract.vertexShader = vs;

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
  _stateAbstract.viewport = _viewport; 
  return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::RSSetFillMode(
    D3D11_FILL_MODE fillMode) {
  // Set the state abstract info
  if (fillMode == D3D11_FILL_SOLID)
    _stateAbstract.fill = kSolid;
  else if (fillMode == D3D11_FILL_WIREFRAME)
    _stateAbstract.fill = kWireFrame;

  _rasterizerDesc.FillMode = fillMode;
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::RSSetCullMode(
    D3D11_CULL_MODE cullMode) {
	// Set the state abstract info
  if (cullMode == D3D11_CULL_NONE)
    _stateAbstract.cull = kNone;
  else if (cullMode == D3D11_CULL_FRONT)
    _stateAbstract.cull = kFront;
  else
    _stateAbstract.cull = kBack;

  _rasterizerDesc.CullMode = cullMode;
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::RSEnableFrontFaceCounterClockwise() {
  // Set the state abstract info
  _stateAbstract.frontClockwise = kFrontCounterClockwise;

  _rasterizerDesc.FrontCounterClockwise = TRUE;
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::RSEnableDepthClipping() {
  // Set the state abstract info
  _stateAbstract.depthClipEnabled = true;

  _rasterizerDesc.DepthClipEnable = TRUE;
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::RSEnableScissor(
    D3D11_RECT scissor) {
  // Set the state abstract info
  _stateAbstract.scissorEnabled = true;
  _stateAbstract.scissor = scissor;

  _rasterizerDesc.ScissorEnable = TRUE;
  _scissor = scissor;
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::RSDisableMultisample() {
  // Set the state abstract info
  _stateAbstract.multisampleCount = kSampleCount_1_Bit;

  _rasterizerDesc.MultisampleEnable = FALSE;
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::RSEnableMultisample(
    UINT samples) {
  if (samples == 1)
    _stateAbstract.multisampleCount = kSampleCount_1_Bit;
  else if (samples == 2)
    _stateAbstract.multisampleCount = kSampleCount_2_Bit;
  else if (samples == 4)
    _stateAbstract.multisampleCount = kSampleCount_4_Bit;
  else if (samples == 8)
    _stateAbstract.multisampleCount = kSampleCount_8_Bit;
  else if (samples == 16)
    _stateAbstract.multisampleCount = kSampleCount_16_Bit;
  else if (samples == 32)
    _stateAbstract.multisampleCount = kSampleCount_32_Bit;
  else
    assert(false, "Unsupported multi-sampling count");

  _rasterizerDesc.MultisampleEnable = TRUE;
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::PSSetPixelShader(Handle ps) {
  _stateAbstract.pixelShader = ps;

  _ps = ps;
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::OMDisableDepthTesting() {
  _stateAbstract.depthEnabled = false;

  _depthStencilDesc.DepthEnable = FALSE;
  _depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::OMEnableDepthTesting(
    D3D11_COMPARISON_FUNC compOp) {
  _stateAbstract.depthEnabled = true;
  _stateAbstract.depthCompOp = (OMDepthCompOp)compOp;

  _depthStencilDesc.DepthEnable = TRUE;
  _depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  _depthStencilDesc.DepthFunc = compOp;
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::OMDisableBlending() {
  _stateAbstract.blendMode = kNoBlend;

  _blendDesc.RenderTarget[0].BlendEnable = FALSE;
  _blendDesc.RenderTarget[0].RenderTargetWriteMask = 0;
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::OMEnableAdditiveBlending() {
  _stateAbstract.blendMode = kAdditiveBlend;

  _blendDesc.RenderTarget[0].BlendEnable = TRUE;
  _blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	_blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	_blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	_blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	_blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	_blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
  _blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::OMEnableAlphaBlending() {
  _stateAbstract.blendMode = kAlphaBlend;

  _blendDesc.RenderTarget[0].BlendEnable = TRUE;
  _blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

  _blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
  _blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_DEST_ALPHA;
  _blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  _blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
  _blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
  _blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::OMSetColorAttachmentFormats(
    std::initializer_list<DXGI_FORMAT> formats) {
  _colorAttachmentFormats = formats;

	return *this;
}

DX::PipelineStateBuilder& DX::PipelineStateBuilder::OMSetDepthAttachmentFormat(
    DXGI_FORMAT format) {
  _depthStencilAttachmentFormat = format;

	return *this;
}

DX::PipelineState* DX::PipelineStateBuilder::Build(class RenderDevice* device) { 
	PipelineState* pipeline = new PipelineState;

	// State abstract
  pipeline->_stateAbstract = _stateAbstract;
	// Topoloty
  pipeline->_topology = _topology;

	const DX::ShaderData& vs = AccessShaderData(_vs);
  const DX::ShaderData& ps = AccessShaderData(_ps);

	// Input layout
	device->GetDevice()->CreateInputLayout(
      _inputLayoutDesc.data(), _inputLayoutDesc.size(), vs.data.data(),
      vs.data.size(), pipeline->_layout.GetAddressOf());
	
	// Viewport
  pipeline->_viewport = _viewport;
	// Scissor
  pipeline->_scissor = _scissor;

	// Rasterizer state
  device->GetDevice()->CreateRasterizerState(
      &_rasterizerDesc, pipeline->_rasterizerState.GetAddressOf());

	// Vertex shader
  device->GetDevice()->CreateVertexShader(vs.data.data(), vs.data.size(), NULL,
                                          pipeline->_vs.GetAddressOf());

	// Pixel shader
  device->GetDevice()->CreatePixelShader(ps.data.data(), ps.data.size(), NULL,
                                         pipeline->_ps.GetAddressOf());

	// Depth-stencil state
  device->GetDevice()->CreateDepthStencilState(
      &_depthStencilDesc, pipeline->_depthState.GetAddressOf());

	// Blend state
  device->GetDevice()->CreateBlendState(&_blendDesc,
                                        pipeline->_blendState.GetAddressOf());

	// Color attachments
  pipeline->_colorAttachmentFormats = _colorAttachmentFormats;

	// Depth stencil attachments
  pipeline->_depthStencilAttachmentFormat = _depthStencilAttachmentFormat;

	return pipeline; 
}
