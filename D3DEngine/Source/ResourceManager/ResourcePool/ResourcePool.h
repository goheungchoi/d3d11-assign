#pragma once

#include "D3DEngine/Core/Geometry.h"
#include "D3DEngine/Core/ShaderData.h"

#include "Utils.h"

#include "Common/Config/Config.h"
#include "Common/UUID/UUID.h"

template <typename T, size_t GROW_SIZE = 1024>
class ResourcePool
{
  std::unordered_map<ns::UUID, uint32_t> _uuidMap;

	HandleTable<T, GROW_SIZE> _handleTable;

public:
  Handle Load(const char* path, void* pReserved)
  {
    ns::UUID uuid = ns::GenerateUUIDFromName(path);

		// Check if the asset is already loaded
    if (auto it = _uuidMap.find(uuid); it == _uuidMap.end())
    {
      // Need to load a new resource
      return LoadImpl(uuid, pReserved);
    }
    else
    {
      // Duplicate the handle
      uint32_t index = it->second;
      Handle handle = _handleTable[index];
      return _handleTable.DuplicateHandle(handle);
    }

    return Handle::kInvalidHandle;
  }

	void DiscardDetailedData(Handle& handle, void* pReserved) {
    DiscardDetailedDataImpl(handle, pReserved);
	}

	void RestoreDetailedData(Handle& handle, void* pReserved) {
    RestoreDetailedDataImpl(handle, pReserved);
	}

  void Unload(Handle& handle, void* pReserved)
  {
    if (_handleTable.GetReferenceCount(handle) == 1)
    {
      UnloadImpl(handle, pReserved);
		}

    // Release the handle
    _handleTable.ReleaseHandle(handle);
  }

  bool IsValidHandle(const Handle& handle) const
  {
    return _handleTable.IsValidHandle(handle);
  }

  const T& AccessResourceData(const Handle& handle) const
  {
    if (!IsValidHandle(handle))
      // TODO: Error message
      throw std::exception("Invalid handle!");

    return _handleTable[handle].value();
  }

private:
  Handle LoadImpl(ns::UUID uuid, void* pReserved)
  {
    return Handle::kInvalidHandle;
  }

	void DiscardDetailedDataImpl(Handle& handle, void* pReserved) {}

	void RestoreDetailedDataImpl(Handle& handle, void* pReserved) {}

	void UnloadImpl(Handle& handle, void* pReserved) { return; }
};

template <>
Handle ResourcePool<DX::ShaderData>::LoadImpl(ns::UUID uuid, void* pUser);
template <>
Handle ResourcePool<DX::TextureData>::LoadImpl(ns::UUID uuid, void* pUser);
template <>
Handle ResourcePool<DX::MaterialData>::LoadImpl(ns::UUID uuid, void* pUser);
template <>
Handle ResourcePool<DX::MeshData>::LoadImpl(ns::UUID uuid, void* pUser);
template <>
Handle ResourcePool<DX::ModelData>::LoadImpl(ns::UUID uuid, void* pUser);


// TODO;
//template <>
//void ResourcePool<TextureData>::UnloadImpl(Handle& uuid, void* pUser);
//template <>
//void ResourcePool<ShaderData>::UnloadImpl(Handle& uuid, void* pUser);
//template <>
//void ResourcePool<MeshData>::UnloadImpl(Handle& uuid, void* pUser);
//template <>
//void ResourcePool<ModelData>::UnloadImpl(Handle& uuid, void* pUser);

// TODO: Restore and discard detailed data.
