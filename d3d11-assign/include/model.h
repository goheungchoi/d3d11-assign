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

struct BoneInfo {
	int id;
	XMMATRIX offset;
};

class Model {
	ID3D11Device* const _device;
	ID3D11DeviceContext* const _context;

	std::vector<Texture> _loadedTextures;

	int _numBones{ 0 };
	std::map<std::string, BoneInfo> _boneInfoMap;

public:

	Model(ID3D11Device* device, ID3D11DeviceContext* context, const char* path);
	~Model();

	void Draw(XMMATRIX topMat);

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
	void ExtractBoneData(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);
};