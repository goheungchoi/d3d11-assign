#pragma once

#include "common.h"

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
class Mesh {
  ID3D11Device* const _device;
  ID3D11DeviceContext* const _context;

  // Mesh data
  std::vector<Vertex> vertices;
  std::vector<Index> indices;
  std::vector<Texture> textures;

  Transform _localTransform{};

  bool _bShouldUpdateModelTransform{true};
  XMMATRIX _modelTransform{XMMatrixIdentity()};

 public:
  Mesh(ID3D11Device* device, ID3D11DeviceContext* context,
       const std::vector<Vertex>& vertices, const std::vector<Index>& indices,
       const std::vector<Texture>& textures) = delete;
  Mesh(ID3D11Device* device, ID3D11DeviceContext* context,
       std::vector<Vertex>&& vertices, std::vector<Index>&& indices,
       std::vector<Texture>&& textures)
      : _device{device},
        _context{context},
        vertices(std::move(vertices)),
        indices(std::move(indices)),
        textures(std::move(textures)) {
    InitPipeline();
    InitBuffers();
  }

  ~Mesh();

  void Draw(XMMATRIX topMat, const std::vector<XMMATRIX>& boneTransforms);

 private:
  ID3D11InputLayout* _inputLayout{nullptr};
  ID3D11VertexShader* _vs{nullptr};
  ID3D11PixelShader* _ps{nullptr};

  bool InitPipeline();

 private:
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
