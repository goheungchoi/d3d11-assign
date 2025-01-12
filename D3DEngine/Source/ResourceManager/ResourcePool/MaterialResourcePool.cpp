#include "ResourcePool.h"

#include "Common/Schema/model_generated.h"
#include <flatbuffers/flatbuffers.h>
#include <fstream>

namespace
{
struct Pools
{
  ResourcePool<DX::TextureData>* texturePool;
};

static Pools* pools;

static ResourcePool<DX::TextureData>* texturePool;
} // namespace

static void ProcessMaterial(DX::MaterialData& mat,
                            const GameResource::Material* geoMat)
{
  mat.name = geoMat->name()->str();

  mat.albedoFactor = {geoMat->albedoFactor()->x(), geoMat->albedoFactor()->y(),
                      geoMat->albedoFactor()->z(), geoMat->albedoFactor()->w()};
	
	// Albedo texture
  DX::TextureType albedoTextureType = DX::TextureType::kAlbedo;
  Handle albedoHandle =
      ::texturePool->Load(geoMat->albedoTexture()->c_str(), &albedoTextureType);
  if (::texturePool->IsValidHandle(albedoHandle))
  {
    mat.albedoTexture = albedoHandle;
	}

  mat.metallicFactor = geoMat->metallicFactor();
  mat.roughnessFactor = geoMat->roughnessFactor();

	// Metallic roughness texture
  DX::TextureType metallicRoughnessTextureType =
      DX::TextureType::kMetalicRoughness;
	Handle metallicRoughnessHandle =
      ::texturePool->Load(geoMat->metallicRoughnessTexture()->c_str(),
                          &metallicRoughnessTextureType);
  if (::texturePool->IsValidHandle(metallicRoughnessHandle))
  {
    mat.metallicRoughnessTexture = metallicRoughnessHandle;
  }

	// Normal texture
  DX::TextureType normalTextureType = DX::TextureType::kNormal;
  Handle normalHandle =
      ::texturePool->Load(geoMat->normalTexture()->c_str(), &normalTextureType);
  if (::texturePool->IsValidHandle(normalHandle))
  {
    mat.normalTexture = normalHandle;
  }

	// Ambient occlusion texture
  DX::TextureType occlusionTextureType = DX::TextureType::kAmbientOcclusion;
  Handle occlusionHandle = ::texturePool->Load(
      geoMat->occlusionTexture()->c_str(), &occlusionTextureType);
  if (::texturePool->IsValidHandle(occlusionHandle))
  {
    mat.occlusionTexture = occlusionHandle;
  }

  mat.emissiveFactor = geoMat->emissiveFactor();
	// Emissive texture
  DX::TextureType emissiveTextureType = DX::TextureType::kEmissive;
  Handle emissiveHandle = ::texturePool->Load(
      geoMat->emissiveTexture()->c_str(), &emissiveTextureType);
  if (::texturePool->IsValidHandle(emissiveHandle))
  {
    mat.emissiveTexture = emissiveHandle;
  }

  mat.alphaMode = static_cast<DX::AlphaMode>(geoMat->alphaMode());
  mat.alphaCutoff = geoMat->alphaCutoff();
  mat.doubleSided = geoMat->doubleSided();
}

template<>
Handle ResourcePool<DX::MaterialData>::LoadImpl(ns::UUID uuid, void* pUser) {
  ::pools = (Pools*)pUser;
  ::texturePool = pools->texturePool;

  fs::path path = GetResourcePath(uuid);

  std::ifstream ifs(path, std::ios::binary);
  std::vector<char> buffer((std::istreambuf_iterator<char>(ifs)),
                           std::istreambuf_iterator<char>());

  const GameResource::Material* geoMat =
      flatbuffers::GetRoot<GameResource::Material>(buffer.data());

  DX::MaterialData mat;
  ProcessMaterial(mat, geoMat);

	return _handleTable.ClaimHandle(std::move(mat));
}
