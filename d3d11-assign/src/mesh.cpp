#include "mesh.h"

Mesh::~Mesh()
{
}

void Mesh::Draw()
{
	/*d3d11DevCon->PSSetShaderResources(0, 1, &CubesTexture);
	d3d11DevCon->PSSetSamplers(0, 1, &CubesTexSamplerState);*/
}

bool Mesh::InitPipeline()
{
	/* Vertex Shader */
	// Compile the vertex shader
	ID3DBlob* vsBlob;
	CHECK(
		CompileShaderFromFile(
			L"shaders/BlinnPhong_VS.hlsl",
			"main",
			"vs_5_0",
			&vsBlob
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
		// Texture coordinate
		D3D11_INPUT_ELEMENT_DESC{
			.SemanticName = "TEXCOORD",
			.SemanticIndex = 0U,
			.Format = DXGI_FORMAT_R32G32_FLOAT,
			.InputSlot = 0,
			.AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT,
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0
		}
	};
	CHECK(
		_device->CreateInputLayout(
			vsInputLayoutDescriptors,
			(UINT)std::size(vsInputLayoutDescriptors),
			vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(),
			&_inputLayout
		)
	);

	// Create the vertex shader object
	CHECK(
		_device->CreateVertexShader(
			vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(),
			NULL,
			&_vs
		)
	);

	// Release the blob
	SafeRelease(&vsBlob);

	/* Pixel Shader */
	ID3DBlob* psBlob;
	CHECK(
		CompileShaderFromFile(
			L"shaders/BlinnPhong_PS.hlsl",
			"main",
			"ps_5_0",
			&psBlob
		)
	);

	// Create the pixel shader object
	CHECK(
		_device->CreatePixelShader(
			psBlob->GetBufferPointer(),
			psBlob->GetBufferSize(),
			NULL,
			&_ps
		)
	);

	// Release the blob
	SafeRelease(&psBlob);

	return true;
}

bool Mesh::InitBuffers()
{
	return false;
}
