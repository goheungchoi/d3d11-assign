#pragma once

#include "D3DEngine/EngineCommon.h"

using type_id = std::uint16_t;

constexpr type_id kInvalidTypeID{(std::numeric_limits<type_id>::max)()};

/**
 * @brief Generate type registry ID,
 * such that `T` is the parent type, and `U` is its child type.
 * @tparam T Parent type
 */
class TypeRegistryBase {
 protected:
  // The ID of the parent type.
  static type_id _baseId;
};

template <typename T>
class TypeRegistryID : public TypeRegistryBase {
  static const type_id _id;

public:
	/**
	 * @brief Return the type ID of the type T.
	 * @return The ID of the T
	 */
	static const type_id Get() { return _id; }
};

template<typename T>
const type_id TypeRegistryID<T>::_id{_baseId++};
