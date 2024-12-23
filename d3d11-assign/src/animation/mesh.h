#pragma once

#include "animation_common.h"

#include "bone.h"

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
class ModelMesh {
 public:
  ID3D11Device* const _device;
  ID3D11DeviceContext* const _context;

  // ModelMesh data
  std::vector<MVertex> vertices;
  std::vector<MIndex> indices;
  std::vector<MTexture> textures;

  MTransform _localTransform{};

  bool _bShouldUpdateModelTransform{true};
  XMMATRIX _modelTransform{
      XMMatrixMultiply(XMMatrixRotationY(PI), XMMatrixTranslation(100, -10, 0))};

 public:
  ModelMesh(ID3D11Device* device, ID3D11DeviceContext* context,
       const std::vector<MVertex>& vertices, const std::vector<MIndex>& indices,
       const std::vector<MTexture>& textures) = delete;
  ModelMesh(ID3D11Device* device, ID3D11DeviceContext* context,
       std::vector<MVertex>&& vertices, std::vector<MIndex>&& indices,
       std::vector<MTexture>&& textures)
      : _device{device},
        _context{context},
        vertices(std::move(vertices)),
        indices(std::move(indices)),
        textures(std::move(textures)) {
    InitPipeline();
    InitBuffers();
  }

  ~ModelMesh();

  // void Draw(XMMATRIX topMat, const std::vector<XMMATRIX>& boneTransforms);

 public:
  ID3D11InputLayout* _inputLayout{nullptr};
  ID3D11VertexShader* _vs{nullptr};
  ID3D11PixelShader* _ps{nullptr};

  bool InitPipeline();

 public:
  ID3D11Buffer* _vbo{nullptr};
  UINT _vbStride{0U};
  UINT _vbOffset{0U};
  UINT _vertexCount{0U};

  ID3D11Buffer* _ibo{nullptr};
  UINT _ibStride{0U};
  UINT _ibOffset{0U};
  UINT _indexCount{0U};

  ID3D11Buffer* _cboPerFrame{nullptr};
  cbPerFrame _cbPerFrame{};
  ID3D11Buffer* _cboPerObject{nullptr};
  cbPerObject _cbPerObject{};

  ID3D11Buffer* _cboMaterialProperties{nullptr};
  cbMaterialProperties _cbMaterialProperties{};

  ID3D11Buffer* _cboLightProperties{nullptr};

  bool InitBuffers();
};
