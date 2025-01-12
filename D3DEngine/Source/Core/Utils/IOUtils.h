#pragma once

#include "D3DEngine/EngineCommon.h"

[[nodiscard]]
std::vector<char> ReadFile(const fs::path& filepath);
