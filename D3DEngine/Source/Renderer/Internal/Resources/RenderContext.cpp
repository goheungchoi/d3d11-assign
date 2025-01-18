#include "RenderContext.h"

#include "Renderer/Internal/Components/RenderPass.h"

void DX::RenderContext::BeginRendering(const RenderPass& pass) {
	// Bind the RS(s) and DS
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

	// TODO: Clear the RS(s) and DS if the pass say so.
	// TODO: loadOp = LOAD_OP_CLAER ?
  const float clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
  for (auto rtv : rtvs) {
    _deferredContext->ClearRenderTargetView(rtv, clearColor);
  }

  if (dsv) {
    _deferredContext->ClearDepthStencilView(
        dsv.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 0.0f, 0);
  }
}

void DX::RenderContext::BindPipelineState(const PipelineState* pipeline) {
  _deferredContext->IASetPrimitiveTopology(pipeline->GetInputTopology());
  _deferredContext->IASetInputLayout(pipeline->GetInputLayout());

	_deferredContext->VSSetShader(pipeline->GetVertexShader(), NULL, 0);

	_deferredContext->RSSetViewports(1, pipeline->GetViewport());
  _deferredContext->RSSetScissorRects(1, pipeline->GetScissor());

	_deferredContext->RSSetState(pipeline->GetRasterizerState());

	_deferredContext->PSSetShader(pipeline->GetPixelShader(), NULL, 0);

	_deferredContext->OMSetDepthStencilState(pipeline->GetDepthStencilState(), 0);
  _deferredContext->OMSetBlendState(pipeline->GetBlendState(), NULL, 0xFFFFFFFF);
}

void DX::RenderContext::DrawMeshBuffer(const DX::MeshBuffer& mesh,
                                       const DX::MaterialInstance& mat) {

	_deferredContext->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(),
                                       &mesh.stride, &mesh.offset);

	_deferredContext->IASetIndexBuffer(mesh.indexBuffer.Get(), mesh.indexFormat, 0);

	// Bind constant buffers
	for (UINT i = 0; i < mat.cbSet.size(); ++i) {
    _deferredContext->VSSetConstantBuffers(i, 1, mat.cbSet[i].GetAddressOf());
	}

	for (UINT i = 0; i < mat.cbSet.size(); ++i) {
    _deferredContext->PSSetConstantBuffers(i, 1, mat.cbSet[i].GetAddressOf());
  }

	// Bind samplers
  for (UINT i = 0; i < mat.samplerSet.size(); ++i) {
    _deferredContext->PSSetSamplers(i, 1, mat.samplerSet[i].GetAddressOf());
	}

	// Bind textures
	for (UINT i = 0; i < mat.textureSet.size(); ++i) {
    _deferredContext->PSSetShaderResources(i, 1, mat.textureSet[i].srv.GetAddressOf());
	}

	_deferredContext->DrawIndexed(mesh.numIndices, 0, 0);
}

void DX::RenderContext::DrawPointLightShadow(
    const DX::MeshBuffer& mesh,
    const DX::PointLightInstance& pointLight, int i) {
  _deferredContext->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(),
                                       &mesh.stride, &mesh.offset);
  _deferredContext->IASetIndexBuffer(mesh.indexBuffer.Get(), mesh.indexFormat,
                                     0);

  // Bind constant buffer
  _deferredContext->VSSetConstantBuffers(
      0, 1, pointLight.pointLightTransforms[i].GetAddressOf());

	_deferredContext->PSSetConstantBuffers(0, 1, pointLight.cameraData.GetAddressOf());

  _deferredContext->DrawIndexed(mesh.numIndices, 0, 0);
}

void DX::RenderContext::DrawPointLightShading(
    const DX::MeshBuffer& lightMesh, const DX::PointLightInstance& pointLight) {
  _deferredContext->IASetVertexBuffers(
      0, 1, lightMesh.vertexBuffer.GetAddressOf(),
                                       &lightMesh.stride, &lightMesh.offset);
  _deferredContext->IASetIndexBuffer(lightMesh.indexBuffer.Get(),
                                     lightMesh.indexFormat,
                                     0);

	// Bind VS constant buffers
  _deferredContext->VSSetConstantBuffers(0, 1,
                                         pointLight.frameData.GetAddressOf());
	_deferredContext->VSSetConstantBuffers(1, 1,
                                         pointLight.objWorld.GetAddressOf());

	// Bind PS constant buffers
  _deferredContext->PSSetConstantBuffers(0, 1,
                                         pointLight.frameData.GetAddressOf());
  _deferredContext->PSSetConstantBuffers(
      2, 1, pointLight.lightShadingConstant.GetAddressOf());

	_deferredContext->Draw(3, 0);
}

void DX::RenderContext::EndRendering() {
  _deferredContext->OMSetRenderTargets(0, NULL, NULL);
}
