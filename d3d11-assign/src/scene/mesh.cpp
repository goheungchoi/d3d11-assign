#include "scene/mesh.h"

void Mesh::Draw()
{
	_cbPerObject.model = GetModelTransform();

	_cbPerFrame.viewProj = XMMatrixTranspose(g_cbPerFrame.viewProj);
	_cbPerObject.model = XMMatrixTranspose(_cbPerObject.model);
	_cbPerObject.modelViewProj = XMMatrixTranspose(_cbPerObject.model * g_cbPerFrame.viewProj);
	/* INPUT ASSEMBLER STAGE */
	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_context->IASetVertexBuffers(0, 1, &_vbo, &_vbStride, &_vbOffset);
	_context->IASetIndexBuffer(_ibo, DXGI_FORMAT_R32_UINT, _ibOffset);
	_context->IASetInputLayout(_inputLayout);

	/* VERTEX STAGE */
	_context->VSSetShader(_vs, nullptr, 0);
	D3D11_MAPPED_SUBRESOURCE cbPerFrameSubresource;
	_context->Map(_cboPerFrame, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &cbPerFrameSubresource);
	memcpy(cbPerFrameSubresource.pData, &_cbPerFrame, sizeof(cbPerFrame));
	_context->Unmap(_cboPerFrame, NULL);
	_context->VSSetConstantBuffers(0, 1, &_cboPerFrame);
	D3D11_MAPPED_SUBRESOURCE cbPerObjectSubresource;
	_context->Map(_cboPerObject, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &cbPerObjectSubresource);
	memcpy(cbPerObjectSubresource.pData, &_cbPerObject, sizeof(cbPerObject));
	_context->Unmap(_cboPerObject, NULL);
	_context->VSSetConstantBuffers(1, 1, &_cboPerObject);

	/* PIXEL STAGE */
	// Diffuse
	_context->PSSetShader(_ps, nullptr, 0);

	// Start sending commands to the gpu.
	_context->DrawIndexed(_indexCount, 0, 0);
}

bool Mesh::InitPipeline()
{
	/* Vertex Shader */
	std::size_t vsByteSize;
	std::vector<uint8_t> vsByteData;
	CHECK(
		ReadBinaryFile(
			L"shaders/VertexShader.cso",
			&vsByteData,
			&vsByteSize
		)
	);

	// Create the input layout of the shader
	// Input layout descriptor
	D3D11_INPUT_ELEMENT_DESC vsInputLayoutDescriptors[] = {
		// Position layout
		D3D11_INPUT_ELEMENT_DESC{
			.SemanticName = "POSITION",
			.SemanticIndex = 0U,
			.Format = DXGI_FORMAT_R32G32B32_FLOAT,
			.InputSlot = 0,
			.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0
		},
		// Texture coordinate
		D3D11_INPUT_ELEMENT_DESC{
			.SemanticName = "COLOR",
			.SemanticIndex = 0U,
			.Format = DXGI_FORMAT_R32G32B32_FLOAT,
			.InputSlot = 0,
			.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0
		},
	};
	CHECK(
		_device->CreateInputLayout(
			vsInputLayoutDescriptors,
			(UINT)std::size(vsInputLayoutDescriptors),
			vsByteData.data(),
			vsByteSize,
			&_inputLayout
		)
	);

	// Create the vertex shader object
	CHECK(
		_device->CreateVertexShader(
			vsByteData.data(),
			vsByteSize,
			NULL,
			&_vs
		)
	);

	/* Pixel Shader */
	std::size_t psByteSize;
	std::vector<uint8_t> psByteData;
	CHECK(
		ReadBinaryFile(
			L"shaders/PixelShader.cso",
			&psByteData,
			&psByteSize
		)
	);

	// Create the pixel shader object
	CHECK(
		_device->CreatePixelShader(
			psByteData.data(),
			psByteSize,
			NULL,
			&_ps
		)
	);

	return true;
}

bool Mesh::InitBuffers()
{
	// Vertex buffer info
	D3D11_BUFFER_DESC vertexBufferInfo{
		.ByteWidth = (UINT)(sizeof(SimpleVertex) * std::size(vertices)),
		.Usage = D3D11_USAGE_IMMUTABLE,		// Only read access from GPU
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,	// Vertex buffer
		.CPUAccessFlags = 0	// No access from CPU
	};

	// Vertex data
	D3D11_SUBRESOURCE_DATA vertexBufferSubresource{
		.pSysMem = vertices.data()
	};

	// Create the vertex buffer.
	CHECK(
		_device->CreateBuffer(
			&vertexBufferInfo,
			&vertexBufferSubresource,
			&_vbo
		)
	);

	// Copy the vertices into the buffer
	// NOTE: You can technically use ID3D11DeviceContext::UpdateSubresource 
	// to copy to a resource with any usage except D3D11_USAGE_IMMUTABLE. 
	// However, we recommend to use ID3D11DeviceContext::UpdateSubresource 
	// to update only a resource with D3D11_USAGE_DEFAULT. 
	// We recommend to use ID3D11DeviceContext::Map and 
	// ID3D11DeviceContext::Unmap to update resources with 
	// D3D11_USAGE_DYNAMIC because that is the specific purpose of 
	// D3D11_USAGE_DYNAMIC resources, and is therefore the most optimized path.
	/*D3D11_MAPPED_SUBRESOURCE vertexBufferSubresource;
	_context->Map(_vbo, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &vertexBufferSubresource);
	memcpy(vertexBufferSubresource.pData, vertices, sizeof(vertices));
	_context->Unmap(_vbo, NULL);*/

	_vbStride = sizeof(SimpleVertex);
	_vbOffset = 0U;
	_vertexCount = static_cast<UINT>(std::size(vertices));

	// Index buffer info
	D3D11_BUFFER_DESC indexBufferDescriptor{
		.ByteWidth = (UINT)(sizeof(Index) * std::size(indices)),
		.Usage = D3D11_USAGE_IMMUTABLE,
		.BindFlags = D3D11_BIND_INDEX_BUFFER,
	};

	// Index buffer data
	D3D11_SUBRESOURCE_DATA indexBufferData{
		.pSysMem = indices.data()
	};

	// Create the index buffer
	CHECK(
		_device->CreateBuffer(
			&indexBufferDescriptor,
			&indexBufferData,
			&_ibo
		)
	);

	_ibStride = sizeof(Index);
	_ibOffset = 0U;
	_indexCount = static_cast<UINT>(std::size(indices));

	// Create the vertex shader constant buffers
	D3D11_BUFFER_DESC cboPerFrameDesc{
		.ByteWidth = sizeof(_cbPerFrame),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE
	};
	CHECK(
		_device->CreateBuffer(
			&cboPerFrameDesc,
			nullptr,
			&_cboPerFrame
		)
	);

	D3D11_BUFFER_DESC cboPerObjectDesc{
		.ByteWidth = sizeof(cbPerObject),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE
	};
	CHECK(
		_device->CreateBuffer(
			&cboPerObjectDesc,
			nullptr,
			&_cboPerObject
		)
	);

	return true;
}
