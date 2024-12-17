#include "model.h"

#include <directxtk/WICTextureLoader.h>

#include <codecvt>
#include <locale>

Model::Model(ID3D11Device* device, ID3D11DeviceContext* context,
             const char* path)
    : _device{device}, _context{context} {
  LoadModel(path);
}

Model::~Model() {}

void Model::Draw(XMMATRIX topMat, const std::vector<XMMATRIX>& boneTransforms) {
  for (Mesh& mesh : _meshes) mesh.Draw(topMat, boneTransforms);
}

void Model::LoadModel(const char* path) {
  Assimp::Importer import;
  const aiScene* scene = import.ReadFile(
      path, aiProcess_Triangulate | aiProcess_ConvertToLeftHanded |
                aiProcess_GenNormals | aiProcess_CalcTangentSpace);

  // Scene loading error
  if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) ||
      !scene->mRootNode) {
    char buf[256];
    sprintf(buf, "ERROR::ASSIMP::%s\n", import.GetErrorString());
    throw std::exception(buf);
  }

  std::string str(path);
  _directory = str.substr(0, str.find_last_of("/\\"));

  ProcessNode(scene->mRootNode, scene);
}

void Model::ProcessNode(aiNode* node, const aiScene* scene) {
  // TODO: Mesh hierarchy
  //
  // Process all the node's meshes
  for (std::size_t i = 0; i < node->mNumMeshes; ++i) {
    aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
    _meshes.push_back(ProcessMesh(mesh, scene));
  }

  // Then do the same for each of its children
  for (std::size_t i = 0; i < node->mNumChildren; ++i) {
    ProcessNode(node->mChildren[i], scene);
  }
}

Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
  std::vector<Vertex> vertices(mesh->mNumVertices);
  std::vector<Index> indices(mesh->mNumFaces * 3);
  std::vector<Texture> textures;

  for (std::size_t i = 0; i < mesh->mNumVertices; i++) {
    Vertex vertex = Vertex::Default();

    // Process vertex position
    Vector3 vector;
    vector.x = mesh->mVertices[i].x;
    vector.y = mesh->mVertices[i].y;
    vector.z = mesh->mVertices[i].z;
    vertex.position = vector;

    // Process vertex texture coordinate
    if (mesh->mTextureCoords[0]) {
      Vector2 uv;
      uv.x = mesh->mTextureCoords[0][i].x;
      uv.y = mesh->mTextureCoords[0][i].y;
      vertex.uv = uv;
    } else {
      vertex.uv = Vector2::Zero;
    }

    // Process vertex normal
    Vector3 normal;
    normal.x = mesh->mNormals[i].x;
    normal.y = mesh->mNormals[i].y;
    normal.z = mesh->mNormals[i].z;
    vertex.normal = normal;

    // Process tangent
    Vector3 tangent;
    tangent.x = mesh->mTangents[i].x;
    tangent.y = mesh->mTangents[i].y;
    tangent.z = mesh->mTangents[i].z;
    vertex.tangent = tangent;

    // Store the vertex
    vertices[i] = vertex;
  }

  // Extract the bone data of the vertices
  ExtractBoneData(vertices, mesh, scene);

  // Process indices
  std::size_t count = 0ULL;
  for (std::size_t i = 0; i < mesh->mNumFaces; ++i) {
    aiFace face = mesh->mFaces[i];
    for (std::size_t j = 0; j < face.mNumIndices; ++j) {
      indices[count++] = face.mIndices[j];
    }
  }

  // Process materials
  if (mesh->mMaterialIndex >= 0) {
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    // Diffuse maps
    std::vector<Texture> diffuseMaps = LoadMaterialTextures(
        material, aiTextureType_DIFFUSE, TEXTURE_TYPE::DIFFUSE, scene);
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    // Specular maps
    std::vector<Texture> specularMaps = LoadMaterialTextures(
        material, aiTextureType_SPECULAR, TEXTURE_TYPE::SPECULAR, scene);
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    // Normal maps
    std::vector<Texture> normalMaps = LoadMaterialTextures(
        material, aiTextureType_NORMALS, TEXTURE_TYPE::NORMALS, scene);
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

    // Height maps
    std::vector<Texture> heightMaps = LoadMaterialTextures(
        material, aiTextureType_HEIGHT, TEXTURE_TYPE::HEIGHT, scene);
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
  }

  return Mesh(_device, _context, std::move(vertices), std::move(indices),
              std::move(textures));
}

std::vector<Texture> Model::LoadMaterialTextures(aiMaterial* material,
                                                 aiTextureType type,
                                                 TEXTURE_TYPE textureType,
                                                 const aiScene* scene) {
  std::vector<Texture> textures;
  for (UINT i = 0; i < material->GetTextureCount(type); ++i) {
    aiString str;
    material->GetTexture(type, i, &str);

    // Check if this texture is already loaded
    bool skip = false;
    for (UINT j = 0; j < _loadedTextures.size(); j++) {
      if (std::strcmp(_loadedTextures[j].path.data(), str.C_Str()) == 0) {
        textures.push_back(_loadedTextures[j]);
        skip = true;
        break;
      }
    }

    if (!skip) {
      Texture texture;

      const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(str.C_Str());
      if (embeddedTexture != nullptr) {
        auto [texturView, samplerState] =
            LoadEmbeddedTexture(_device, _context, embeddedTexture);
        texture.textureView = texturView;
        texture.samplerState = samplerState;
      } else {
        auto [texturView, samplerState] =
            CreateTextureFromFile(_device, _context, str.C_Str(), _directory);
        texture.textureView = texturView;
        texture.samplerState = samplerState;
      }

      texture.type = textureType;
      texture.path = str.C_Str();
      textures.push_back(texture);
      _loadedTextures.push_back(texture);
    }
  }

  return textures;
}

void Model::ExtractBoneData(std::vector<Vertex>& vertices, aiMesh* mesh,
                            const aiScene* scene) {
  for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
    aiBone* currBone = mesh->mBones[boneIndex];

    int boneID = -1;
    std::string boneName{currBone->mName.C_Str()};

    // Find if the bone already exists in the bone info map
    if (_boneInfoMap.find(boneName) == _boneInfoMap.end()) {
      // When it's not found in the map,
      // Add a new bone info
      BoneInfo boneInfo;
      boneInfo.id = _numBones;
      boneInfo.offset =
          XMMatrixTranspose(XMMATRIX(&currBone->mOffsetMatrix.a1));
      _boneInfoMap[boneName] = boneInfo;
      boneID = _numBones;
      _numBones++;
    } else {
      // When it's found, that means this bone affects
      // vertices of mesh out of its scope, so take it's Id
      // and proceed further to know which vertices it affects
      boneID = _boneInfoMap[boneName].id;
    }

    // Find which vertices are affected by this current bone
    auto weights = currBone->mWeights;
    int numWeights = currBone->mNumWeights;
    for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
      int vertexId = weights[weightIndex].mVertexId;
      float weight = weights[weightIndex].mWeight;
      vertices[vertexId].SetBoneData(boneID, weight);
    }
  }
}

D3D11TextureDataPair LoadEmbeddedTexture(ID3D11Device* const device,
                                         ID3D11DeviceContext* const context,
                                         const aiTexture* embeddedTexture) {
  HRESULT hr;
  ID3D11ShaderResourceView* texture = nullptr;

  if (embeddedTexture->mHeight != 0) {
    // Load an uncompressed ARGB8888 embedded texture
    D3D11_TEXTURE2D_DESC desc;
    desc.Width = embeddedTexture->mWidth;
    desc.Height = embeddedTexture->mHeight;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA subresourceData;
    subresourceData.pSysMem = embeddedTexture->pcData;
    subresourceData.SysMemPitch = embeddedTexture->mWidth * 4;
    subresourceData.SysMemSlicePitch =
        embeddedTexture->mWidth * embeddedTexture->mHeight * 4;

    ID3D11Texture2D* texture2D = nullptr;
    hr = device->CreateTexture2D(&desc, &subresourceData, &texture2D);
    if (FAILED(hr)) throw std::exception("CreateTexture2D failed!");
    hr = device->CreateShaderResourceView(texture2D, nullptr, &texture);
    if (FAILED(hr)) throw std::exception("CreateShaderResourceView failed!");

    // Sampler state
    ID3D11SamplerState* samplerState{nullptr};

    // Sampler description
    D3D11_SAMPLER_DESC samplerDesc{};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

    // Create the sampler state
    device->CreateSamplerState(&samplerDesc, &samplerState);

    return {texture, samplerState};
  }

  // mHeight is 0, so try to load a compressed texture of mWidth bytes
  const size_t size = embeddedTexture->mWidth;

  hr = CreateWICTextureFromMemory(
      device, context,
      reinterpret_cast<const unsigned char*>(embeddedTexture->pcData), size,
      nullptr, &texture);
  if (FAILED(hr))
    throw std::exception("Texture couldn't be created from memory!");

  // Sampler state
  ID3D11SamplerState* samplerState{nullptr};

  // Sampler description
  D3D11_SAMPLER_DESC samplerDesc{};
  samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  samplerDesc.MinLOD = 0;
  samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

  // Create the sampler state
  device->CreateSamplerState(&samplerDesc, &samplerState);

  return {texture, samplerState};
}

D3D11TextureDataPair CreateTextureFromFile(ID3D11Device* const device,
                                           ID3D11DeviceContext* const context,
                                           const char* path,
                                           const std::string& _directory,
                                           bool gamma) {
  HRESULT res = S_OK;

  std::string filename(path);
  filename = _directory + '/' + filename;

  // Texture view
  ID3D11ShaderResourceView* textureView{nullptr};

  // Convert the char type filename to wchar_t type filename.
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  std::wstring wfilename = converter.from_bytes(filename);

  // Create the texture
  res = DirectX::CreateWICTextureFromFile(device, wfilename.c_str(), nullptr,
                                          &textureView);

  // Sampler state
  ID3D11SamplerState* samplerState{nullptr};

  // Sampler description
  D3D11_SAMPLER_DESC samplerDesc{};
  samplerDesc.Filter =
      D3D11_FILTER_MIN_MAG_MIP_LINEAR;  // Linear filtering method
  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
  samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  samplerDesc.MinLOD = 0;
  samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

  // Create the sampler state
  device->CreateSamplerState(&samplerDesc, &samplerState);

  return {textureView, samplerState};
}
