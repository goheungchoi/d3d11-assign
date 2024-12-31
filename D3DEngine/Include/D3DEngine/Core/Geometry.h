#pragma once

#include "D3DEngine/EngineCommon.h"

#include "D3DEngine/Core/Handle.h"

#include <directxtk/SimpleMath.h>
using namespace DirectX;
using namespace DirectX::SimpleMath;

#include <dxgiformat.h>

namespace DX {

enum class TextureType : uint8_t {
  kUnknown = 0x00,
  kDiffuse,
  kSpecular,
  kAmbient,
  kEmissive,
  kHeight,
  kShininess,
  kOpacity,
  kDisplacement,

  kAlbedo,
  kNormal,
  kMetalic,
  kRoughness,
  kMetalicRoughness,
  kAmbientOcclusion,

  kInvalidType = 0xFF
};

struct TextureData {
  TextureType type;

  bool isSRGB;
  bool isCubeMap;

  DXGI_FORMAT format;

  uint32_t width;
  uint32_t height;

  uint32_t mipLevels;
  uint32_t arrayLayers;  // Cube map has 6 layers

  std::vector<uint8_t> ddsData;
};

enum class MaterialPassType { kUnknown, kOpacity, kTransparent };

enum class AlphaMode {
  kOpaque,  // Alpha value is ignored
  kMask,    // Alpha cutoff
  kBlend    // Blended with the background
};

struct MaterialData {
  // MaterialPassType passType;
  std::string name;

  Color albedoFactor;
  Handle albedoTexture;

  float metallicFactor;
  float roughnessFactor;
  Handle metallicRoughnessTexture;

  Handle normalTexture;

  Handle occlusionTexture;

  float emissiveFactor;
  Handle emissiveTexture;

  AlphaMode alphaMode;
  float alphaCutoff;
  bool doubleSided;
};

struct Vertex {
  Vector3 position;
  Vector3 normal;
  Vector3 tangent;
  Vector3 bitangent;
  Vector2 texcoord;
  Vector4 color;
};

using Index = uint32_t;

struct Face {
  uint32_t v1, v2, v3;
};

enum class MeshMobility {
  kImmobile,  // Non-movable
  kMobile     // Movable
};

enum class MeshMutability {
  kImmutable,  // Non-deformable
  kMutable     // Deformable like skinning and clothes
};

enum class MeshConstancy { kInconstant, kConstant };

struct MeshData {
  std::vector<Vertex> vertices;
  std::vector<Index> indices;

	Handle material;
};

// Light
enum class LightType : uint32_t { kUndefined = 0, kDirectional, kPoint, kSpot };

struct LightData {
  Vector4 components;  // [x,y,z,1] - position, [x,y,z,0] - direction
  Vector4 radiance;    // r, g, b, intensity

  float spotAngle;
  float constantAttenuation;
  float linearAttenuation;
  float quadraticAttenuation;

  LightType type;
  bool enabled;
};

}

