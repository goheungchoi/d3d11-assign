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

	int _numBones{ 0 };
	std::unordered_map<std::string, BoneInfo> _boneInfoMap;

public:

	Model(ID3D11Device* device, ID3D11DeviceContext* context, const char* path);
	~Model();

	auto& GetBoneInfoMap() { return _boneInfoMap; }
	int& GetNumBones() { return _numBones; }

	void Draw(XMMATRIX topMat, const std::vector<XMMATRIX>& boneTransforms);

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
	/**
	 * @brief Extract animation bone information from a scene.
	 * @param vertices Vertices of a model to set bone weights
	 * @param mesh Assimp mesh that contains the bone data
	 * @param scene ?
	 */
	void ExtractBoneData(std::vector<Vertex>& vertices, aiMesh* mesh, const aiScene* scene);
};