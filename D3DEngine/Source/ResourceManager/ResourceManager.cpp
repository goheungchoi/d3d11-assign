#include "D3DEngine/ResourceManager/ResourceManager.h"

// TODO: Goheung Choi
#include "ResourcePool/ResourcePool.h"

struct __ResourceManagerPrivate__ {
  ResourcePool<DX::TextureData> texturePool;
  ResourcePool<DX::ShaderData> shaderPool;
  ResourcePool<DX::MeshData> meshPool;
  ResourcePool<DX::MaterialData> materialPool;
  ResourcePool<DX::ModelData> modelPool;
};

const fs::path kResourceDir{};

static __ResourceManagerPrivate__& _m() {
  static __ResourceManagerPrivate__ _resourceManagerPrivate;
  return _resourceManagerPrivate;
}

static Handle __LoadTexture__(const std::string& path,
                                     DX::TextureType type) {
  return Handle();
}
static const DX::TextureData& __AccessTextureData__(Handle handle) {
  return DX::TextureData();
}

static void __UnloadTexture__(Handle& handle) {}

static Handle __LoadShader__(const std::string& path, DX::ShaderType type) {
  return Handle();
}
static const DX::ShaderData& __AccessShaderData__(Handle handle) {
  return DX::ShaderData();
}
static void __UnloadShader__(Handle handle) {}
static Handle __LoadMesh__(const std::string& path) { return Handle(); }
static const DX::MeshData& __AccessMeshData__(Handle handle) { return DX::MeshData(); }
static void __UnloadMesh__(Handle handle) {}

static Handle __LoadMaterial__(const std::string& path) {
  // TODO:
  // Load the material data

  // Load textures used in the material

  return Handle();
}
static const DX::MaterialData& __AccessMaterialData__(Handle handle) {
  return DX::MaterialData();
}
static void __UnloadMaterial__(Handle handle) {}
static ResourceType __GetResourceType__(const Handle& handle) {
  return ResourceType::kUnknown;
}
static bool __IsValidHandle__(const Handle& handle) { return false; }

static void __UnloadAll__() {}

const ResourceManager* GetResourceManager() {
  static ResourceManager _resourceManager{
      .LoadTexture = __LoadTexture__,
      .AccessTextureData = __AccessTextureData__,
      .UnloadTexture = __UnloadTexture__,

      .LoadShader = __LoadShader__,
      .AccessShaderData = __AccessShaderData__,
      .UnloadShader = __UnloadShader__,

      .LoadMesh = __LoadMesh__,
      .AccessMeshData = __AccessMeshData__,
      .UnloadMesh = __UnloadMesh__,

      .LoadMaterial = __LoadMaterial__,
      .AccessMaterialData = __AccessMaterialData__,
      .UnloadMaterial = __UnloadMaterial__,

      .GetResourceType = __GetResourceType__,
      .IsValidHandle = __IsValidHandle__,

      .UnloadAll = __UnloadAll__};

  return &_resourceManager;
}
