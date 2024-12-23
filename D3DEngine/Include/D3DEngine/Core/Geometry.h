#pragma once

#include "D3DEngine/EngineCommon.h"

#include <directxtk/SimpleMath.h>
using namespace DirectX;
using namespace DirectX::SimpleMath;

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
	kImmobile,	// Non-movable
	kMobile			// Movable
};

enum class MeshMutability {
	kImmutable,	// Non-deformable
	kMutable		// Deformable like skinning and clothes
};

enum class MeshConstancy {
	kInconstant,
	kConstant
};

struct Mesh {
  std::vector<Vertex> vertices;
  std::vector<Index> indices;
};



