#pragma once 

#include "common.h"

#include <d3d11_1.h>

#include <directxtk/SimpleMath.h>
using namespace DirectX;
using namespace DirectX::SimpleMath;

#define MAX_LIGHTS 1

#define MAX_BONES 100
#define MAX_BONE_WEIGHTS 8

// MVP Transform Constant buffer
struct cbPerFrame {
  Matrix viewProj;
};

struct cbPerObject {
  Matrix model;
  Matrix inverseTransposeModel;
  //-----------------------
  Matrix boneTransforms[MAX_BONES];  // 64 x 100 = 6400 bytes
};

// Material
struct _Material {
  Vector4 emissive;
  Vector4 ambient;
  Vector4 diffuse;
  Vector4 specular;
  //-----------------------
  float shininess;
  uint32_t useTexture;
  float padding[2];
};

__declspec(align(16)) 
struct cbMaterialProperties {
  _Material material;
};

struct _Light {
  Vector4 position;  // 16 bytes
  //----------------------------------- (16 byte boundary)
  Vector4 direction;  // 16 bytes
  //----------------------------------- (16 byte boundary)
  Vector4 color;  // 16 bytes
  //----------------------------------- (16 byte boundary)
  float spotAngle;  // 4 bytes
  float constAtt;   // 4 bytes
  float linearAtt;  // 4 bytes
  float quadAtt;    // 4 bytes
  //----------------------------------- (16 byte boundary)
  uint32_t lightType;  // 4 bytes
  uint32_t enabled;    // 4 bytes
  float padding[2];    // 8 bytes
                       //----------------------------------- (16 byte boundary)
};  // Total: 80

__declspec(align(16)) 
struct cbLightProperties {
  Vector4 eyePosition;
  Vector4 globalAmbient;
  _Light lights[MAX_LIGHTS];
};

// Transform struct
struct MTransform {
  // Local space transform
  XMVECTOR scale;
  XMVECTOR rotate;  // Quaternion rotation
  XMVECTOR translate;
};

// Vertex struct
struct MVertex {
  Vector3 position;
  Vector2 uv;
  Vector3 normal;
  Vector3 tangent;
  //-----------------------
  int boneIDs[MAX_BONE_WEIGHTS];
  float boneWeights[MAX_BONE_WEIGHTS];

  static constexpr MVertex Default() {
    MVertex v{};
    for (int i = 0; i < MAX_BONE_WEIGHTS; ++i) {
      v.boneIDs[i] = -1;
      v.boneWeights[i] = 0.f;
    }
    return v;
  }

  void SetBoneData(int boneID, float weight) {
    for (int i = 0; i < MAX_BONE_WEIGHTS; ++i) {
      // Find the next empty spot
      if (boneIDs[i] < 0) {
        boneIDs[i] = boneID;
        boneWeights[i] = weight;
        break;
      }
    }
  }
};

using MIndex = uint32_t;

// Texture Types
enum class TEXTURE_TYPE {
  NONE,
  DIFFUSE,
  SPECULAR,
  AMBIENT,
  EMISSIVE,
  NORMALS,
  HEIGHT,
  OPACITY,
  DISPLACEMENT,
  LIGHTMAP,
  REFLECTION,

  /* PBR Materials */
  ALBEDO,
  METALICNESS,
  DIFFUSE_ROUGHNESS,
  AMBIENT_OCCLUSION,
  EMISSIVE_COLOR,
  NORMAL_CAMERA,
};

// Texture struct
struct MTexture {
  ID3D11ShaderResourceView* textureView;
  ID3D11SamplerState* samplerState;
  TEXTURE_TYPE type;
  std::string path;  // Texture equality check; TODO: need optimization
};

std::vector<uint8_t> CompileShaderFromFile(const WCHAR* filename,
                                           LPCSTR entryPoint,
                                           LPCSTR shaderModel);

// Utility function to read the data and size of a binary file
HRESULT ReadBinaryFile(const WCHAR* filename, std::vector<uint8_t>* data,
                       std::size_t* size);