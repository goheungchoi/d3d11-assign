#pragma once

#include "common.h"

#include "d3d_utility.h"

#include "mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using D3D11TextureDataPair = std::pair<ID3D11ShaderResourceView*, ID3D11SamplerState*>;

D3D11TextureDataPair LoadEmbeddedTexture(
	ID3D11Device* const device,
	ID3D11DeviceContext* const context,
	const aiTexture* embeddedTexture
);

D3D11TextureDataPair CreateTextureFromFile(
	ID3D11Device* const device,
	ID3D11DeviceContext* const context,
	const char* path,
	const std::string& directory, 
	bool gamma = false
);

class Model {
	ID3D11Device* const _device;
	ID3D11DeviceContext* const _context;

	std::vector<Texture> _loadedTextures;

	XMMATRIX _modelTransform{ XMMatrixIdentity() };

public:

	Model(ID3D11Device* device, ID3D11DeviceContext* context, const char* path);
	~Model();

	void Draw(XMMATRIX topMat);

	void Scale(float scale) {
		_modelTransform *= XMMatrixScaling(scale, scale, scale);
	}

	void Translate(float dx, float dy, float dz) {
		_modelTransform *= XMMatrixTranslation(dx, dy, dz);
	}

private:
	std::vector<Mesh> _meshes;
	std::string _directory;

	void LoadModel(const char* path);
	void ProcessNode(aiNode* node, const aiScene* scene);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> LoadMaterialTextures(
		aiMaterial* material,
		aiTextureType type,
		TEXTURE_TYPE textureType,
		const aiScene* scene
	);
};