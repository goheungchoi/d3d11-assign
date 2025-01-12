#include "D3DEngine/ResourceManager/ResourceManager.h"

// TODO: Goheung Choi
#include "ResourcePool/ResourcePool.h"

namespace
{
struct __ResourceManagerPrivate__
{
  ResourcePool<DX::ShaderData> shaderPool;
  ResourcePool<DX::TextureData> texturePool;
  ResourcePool<DX::MaterialData> materialPool;
  ResourcePool<DX::MeshData> meshPool;
  ResourcePool<DX::ModelData> modelPool;
};

struct Pools
{
  ResourcePool<DX::TextureData>* texturePool;
  ResourcePool<DX::MaterialData>* materialPool;
  ResourcePool<DX::MeshData>* meshPool;
};
}


static ::__ResourceManagerPrivate__& _m() {
  static ::__ResourceManagerPrivate__ _resourceManagerPrivate;
  return _resourceManagerPrivate;
}

static ::Pools& _pools() {
  static ::Pools pools{&_m().texturePool, &_m().materialPool, &_m().meshPool};
  return pools;
}

static Handle __LoadShader__(const std::string& path, DX::ShaderType type) {
  return Handle();
}
static const DX::ShaderData& __AccessShaderData__(Handle handle) {
  return DX::ShaderData();
}
static void __UnloadShader__(Handle handle) {}

static Handle __LoadTexture__(const std::string& path, DX::TextureType type) {
  Handle textureHandle = _m().texturePool.Load(path.c_str(), &type);
  if (_m().texturePool.IsValidHandle(textureHandle))
  {
    return textureHandle;
  }
  return Handle();
}
static const DX::TextureData& __AccessTextureData__(Handle handle) {
  return _m().texturePool.AccessResourceData(handle);
}
static void __UnloadTexture__(Handle& handle) {
  _m().texturePool.Unload(handle, nullptr);
}

static Handle __LoadMaterial__(const std::string& path) {
  Handle matHandle = _m().materialPool.Load(path.c_str(), &_pools());
  if (_m().materialPool.IsValidHandle(matHandle))
  {
    return matHandle;
  }
  return Handle();
}
static const DX::MaterialData& __AccessMaterialData__(Handle handle) {
  return _m().materialPool.AccessResourceData(handle);
}
static void __UnloadMaterial__(Handle handle) {
	// TODO;
}

static Handle __LoadMesh__(const std::string& path) {
  Handle meshHandle = _m().meshPool.Load(path.c_str(), &_pools());
  if (_m().meshPool.IsValidHandle(meshHandle))
  {
    return meshHandle;
	}
  return Handle();
}
static const DX::MeshData& __AccessMeshData__(Handle handle) {
  return _m().meshPool.AccessResourceData(handle);
}
static void __UnloadMesh__(Handle handle) {
	// TODO:
}

static Handle __LoadModel__(const std::string& path) {
  Handle modelHandle = _m().modelPool.Load(path.c_str(), &_pools());
  if (_m().modelPool.IsValidHandle(modelHandle))
  {
    return modelHandle;
	}
  return Handle();
}
static const DX::ModelData& __AccessModelData__(Handle handle) {
  return _m().modelPool.AccessResourceData(handle);
}
static void __UnloadModel__(Handle handle) {
	// TODO:
}

static DX::ResourceType __GetResourceType__(const Handle& handle) {
  // TODO:
  return DX::ResourceType::kUnknown;
}
static bool __IsValidHandle__(const Handle& handle) {
	// TODO:
  return false;
}

static void __UnloadAll__() {
  // TODO:
}


const DX::ResourceManager* GetResourceManager() { 
	static DX::ResourceManager _resourceManager{

      .LoadShader = __LoadShader__,
      .AccessShaderData = __AccessShaderData__,
      .UnloadShader = __UnloadShader__,

      .LoadTexture = __LoadTexture__,
      .AccessTextureData = __AccessTextureData__,
      .UnloadTexture = __UnloadTexture__,

      .LoadMaterial = __LoadMaterial__,
      .AccessMaterialData = __AccessMaterialData__,
      .UnloadMaterial = __UnloadMaterial__,

      .LoadMesh = __LoadMesh__,
      .AccessMeshData = __AccessMeshData__,
      .UnloadMesh = __UnloadMesh__,

			.LoadModel = __LoadModel__,
			.AccessModelData = __AccessModelData__,
			.UnloadModel = __UnloadModel__,

      .GetResourceType = __GetResourceType__,
      .IsValidHandle = __IsValidHandle__,

      .UnloadAll = __UnloadAll__};
	
	return &_resourceManager;
}


