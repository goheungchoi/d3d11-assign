#include "mesh.h"

Mesh::~Mesh()
{
}

void Mesh::Draw(XMMATRIX topMat)
{
	_cbPerFrame.viewProj = XMMatrixTranspose(topMat);
	_cbPerObject.model = XMMatrixTranspose(_cbPerObject.model);

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
	_context->PSSetShaderResources(0, 1, &textures[0].textureView);
	_context->PSSetSamplers(0, 1, &textures[0].samplerState);
	// Specular
	_context->PSSetShaderResources(1, 1, &textures[1].textureView);
	_context->PSSetSamplers(1, 1, &textures[1].samplerState);
	// Normal
	_context->PSSetShaderResources(2, 1, &textures[2].textureView);
	_context->PSSetSamplers(2, 1, &textures[2].samplerState);
	// Bind constant buffers
	// Material properties
	D3D11_MAPPED_SUBRESOURCE cbMaterialPropertiesSubresource;
	_context->Map(_cboMaterialProperties, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &cbMaterialPropertiesSubresource);
	memcpy(cbMaterialPropertiesSubresource.pData, &_cbMaterialProperties, sizeof(cbMaterialProperties));
	_context->Unmap(_cboMaterialProperties, NULL);
	_context->PSSetConstantBuffers(0, 1, &_cboMaterialProperties);
	// Light properties
	D3D11_MAPPED_SUBRESOURCE cbLightPropertiesSubresource;
	_context->Map(_cboLightProperties, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &cbLightPropertiesSubresource);
	memcpy(cbLightPropertiesSubresource.pData, &g_lightProperties, sizeof(cbLightProperties));
	_context->Unmap(_cboLightProperties, NULL);
	_context->PSSetConstantBuffers(1, 1, &_cboLightProperties);

	// Start sending commands to the gpu.
	_context->DrawIndexed(_indexCount, 0, 0);
}

bool Mesh::InitPipeline()
{
	/* Vertex Shader */
	// Compile the vertex shader
	/*ID3DBlob* vsBlob;
	CHECK(
		CompileShaderFromFile(
			L"shaders/BlinnPhong_VS.hlsl",
			"main",
			"vs_5_0",
			&vsBlob
		)
	);*/

	std::size_t vsByteSize;
	std::vector<uint8_t> vsByteData;
	CHECK(
		ReadBinaryFile(
			L"shaders/BlinnPhong_VS.cso",
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
			.SemanticName = "TEXCOORD",
			.SemanticIndex = 0U,
			.Format = DXGI_FORMAT_R32G32_FLOAT,
			.InputSlot = 0,
			.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0
		},
		// Normal layout
		D3D11_INPUT_ELEMENT_DESC{
			.SemanticName = "NORMAL",
			.SemanticIndex = 0U,
			.Format = DXGI_FORMAT_R32G32B32_FLOAT,
			.InputSlot = 0,
			.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0
		},
		// Tangent layout
		D3D11_INPUT_ELEMENT_DESC{
			.SemanticName = "TANGENT",
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

	//// Release the blob
	//SafeRelease(&vsBlob);

	/* Pixel Shader */
	/*ID3DBlob* psBlob;
	CHECK(
		CompileShaderFromFile(
			L"shaders/BlinnPhong_PS.hlsl",
			"main",
			"ps_5_0",
			&psBlob
		)
	);*/

	std::size_t psByteSize;
	std::vector<uint8_t> psByteData;
	CHECK(
		ReadBinaryFile(
			L"shaders/BlinnPhong_PS.cso",
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

	// Release the blob
	//SafeRelease(&psBlob);

	return true;
}

bool Mesh::InitBuffers()
{
	// Vertex buffer info
	D3D11_BUFFER_DESC vertexBufferInfo{
		.ByteWidth = (UINT)(sizeof(Vertex) * std::size(vertices)),
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

	_vbStride = sizeof(Vertex);
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

	// Create the pixel shader constant buffers
	D3D11_BUFFER_DESC cboMaterialPropertiesDesc{
		.ByteWidth = sizeof(cbMaterialProperties),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE
	};
	CHECK(
		_device->CreateBuffer(
			&cboMaterialPropertiesDesc,
			nullptr,
			&_cboMaterialProperties
		)
	);

	D3D11_BUFFER_DESC cboLightPropertiesDesc{
		.ByteWidth = sizeof(cbLightProperties),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE
	};
	CHECK(
		_device->CreateBuffer(
			&cboLightPropertiesDesc,
			nullptr,
			&_cboLightProperties
		)
	);

	// NOTE: Hard-coded material properties
	// Need update dynamically.
	_cbMaterialProperties.material.shininess = 128.f;
	_cbMaterialProperties.material.useTexture = true;

	return true;
}