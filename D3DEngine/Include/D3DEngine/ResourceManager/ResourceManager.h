#pragma once

#include "D3DEngine/Core/Geometry.h"
#include "D3DEngine/Core/ShaderData.h"

namespace DX {
enum class ResourceType : uint16_t {
  kUnknown = 0x0,
  kShader,
  kMesh,
  kMaterial,
  kTexture,
  kModel,
  kAnimation,
  kAnimator,
  kAudio,

  kInvalid = 0xFFFF
};

struct ResourceManager {
  Handle (*LoadShader)(const std::string& path, ShaderType type);
  const ShaderData& (*AccessShaderData)(Handle handle);
  void (*UnloadShader)(Handle handle);

  Handle (*LoadTexture)(const std::string& path, TextureType type);
  const TextureData& (*AccessTextureData)(Handle handle);
  void (*UnloadTexture)(Handle& handle);

  Handle (*LoadMaterial)(const std::string& path);
  const MaterialData& (*AccessMaterialData)(Handle handle);
  void (*UnloadMaterial)(Handle handle);

  Handle (*LoadMesh)(const std::string& path);
  const MeshData& (*AccessMeshData)(Handle handle);
  void (*UnloadMesh)(Handle handle);

  Handle (*LoadModel)(const std::string& path);
  const ModelData& (*AccessModelData)(Handle handle);
  void (*UnloadModel)(Handle handle);

  ResourceType (*GetResourceType)(const Handle& handle);
  bool (*IsValidHandle)(const Handle& handle);

  void (*UnloadAll)();
};

const ResourceManager* GetResourceManager();

inline Handle LoadShader(const std::string& path, ShaderType type) {
  return GetResourceManager()->LoadShader(path, type);
}

inline const ShaderData& AccessShaderData(Handle handle) {
  return GetResourceManager()->AccessShaderData(handle);
}

inline void UnloadShader(Handle handle) {
  GetResourceManager()->UnloadShader(handle);
}

inline Handle LoadTexture(const std::string& path, TextureType type) {
  return GetResourceManager()->LoadTexture(path, type);
}

inline const TextureData& AccessTextureData(Handle handle) {
  return GetResourceManager()->AccessTextureData(handle);
}

inline void UnloadTexture(Handle handle) {
  GetResourceManager()->UnloadTexture(handle);
}

inline Handle LoadMaterial(const std::string& path) {
  return GetResourceManager()->LoadMaterial(path);
}

inline const MaterialData& AccessMaterialData(Handle handle) {
  return GetResourceManager()->AccessMaterialData(handle);
}

inline void UnloadMaterial(Handle handle) {
  GetResourceManager()->UnloadMaterial(handle);
}

inline Handle LoadMesh(const std::string& path) {
  return GetResourceManager()->LoadMesh(path);
}

inline const MeshData& AccessMeshData(Handle handle) {
  return GetResourceManager()->AccessMeshData(handle);
}

inline void UnloadMesh(Handle handle) {
  GetResourceManager()->UnloadMesh(handle);
}

/**
 * @brief Load the model
 * @param path The relative path to the model data from the Asset directory.
 * @return Invalid handle if not available, otherwise, returns a valid handle.
 */
inline Handle LoadModel(const std::string& path) {
  return GetResourceManager()->LoadModel(path);
}

inline const ModelData& AccessModelData(Handle handle) {
  return GetResourceManager()->AccessModelData(handle);
}

inline void UnloadModel(Handle handle) {
  GetResourceManager()->UnloadModel(handle);
}

inline ResourceType GetResourceType(const Handle& handle) {
  GetResourceManager()->GetResourceType(handle);
}

inline bool IsValidHandle(const Handle& handle) {
  return GetResourceManager()->IsValidHandle(handle);
}

inline void UnloadAll() { GetResourceManager()->UnloadAll(); }
}  // namespace DX
