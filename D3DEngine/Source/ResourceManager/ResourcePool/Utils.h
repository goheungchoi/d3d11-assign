#pragma once

#include "Common/Config/Config.h"

#include <string>
#include <filesystem>
namespace fs = std::filesystem;

#include "Common/UUID/UUID.h"

inline fs::path GetResourcePath(ns::UUID uuid) {
	std::string strUUID = uuid.ToString();
	return fs::path(kResourceDir) / strUUID.substr(0, 2) / strUUID;
}
