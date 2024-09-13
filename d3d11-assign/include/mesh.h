#pragma once

#include "common.h"

#include "d3d_utility.h"

/**
 * @brief A simple mesh class.
 * - Load a model.
 * - Set vertices, normal, and uv.
 * - Set vertex indices.
 * - Set local transform.
 * - Bind a texture.
 * - Build a mesh hierarchy
 *
 */
class Mesh {
	ID3D11Device* const _device;
	ID3D11DeviceContext* const _context;

	// Mesh data
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	std::vector<Texture> textures;

	Transform transform{};

public:
	Mesh(ID3D11Device* device,
		ID3D11DeviceContext* context,
		const std::vector<Vertex>& vertices,
		const std::vector<uint32_t>& indices,
		const std::vector<Texture>& textures) = delete;
	Mesh(ID3D11Device* device,
		ID3D11DeviceContext* context, 
		std::vector<Vertex>&& vertices,
		std::vector<uint32_t>&& indices,
		std::vector<Texture>&& textures) :
		_device{ device },
		_context{ context },
		vertices(std::move(vertices)),
		indices(std::move(indices)),
		textures(std::move(textures)) {}

	~Mesh();

	void Draw(/* Renderer */);

private:

	ID3D11InputLayout* _inputLayout{ nullptr };
	ID3D11VertexShader* _vs{ nullptr };
	ID3D11PixelShader* _ps{ nullptr };

	bool InitPipeline();

private:
	ID3D11Buffer* _vbo{ nullptr };
	UINT _vertexBufferStride{ 0U };
	UINT _vertexBufferOffset{ 0U };
	UINT _vertexCount{ 0U };

	ID3D11Buffer* _ibo{ nullptr };
	UINT _indexBufferStride{ 0U };
	UINT _indexBufferOffset{ 0U };
	UINT _indexCount{ 0U };

	ID3D11Buffer* _cbo{ nullptr };
	ConstantBuffer _cbData{};

	bool InitBuffers();
};
