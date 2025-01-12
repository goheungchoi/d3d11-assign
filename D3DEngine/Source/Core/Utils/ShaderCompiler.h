#pragma once

#include "D3DEngine/Core/ShaderData.h"

namespace DX {

bool PopulateShaderVariants(
    size_t numOptions, const ShaderOption* defineOptions,
    std::vector<std::vector<ShaderDefine>>& outVariants);

bool GenerateShaderVariantsFile(const char* shaderDir);

}
