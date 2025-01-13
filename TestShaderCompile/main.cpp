// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
#define NOGDI
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

// STL Libraries
#include <iostream>
#include <memory>
#include <utility>
#include <typeindex>
#include <typeinfo>
#include <any>
#include <optional>
#include <functional>
#include <concepts>
#include <numbers>
#include <limits>
#include <algorithm>

#include <filesystem>
namespace fs = std::filesystem;
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
using namespace std::literals;
#include <format>

enum class ShaderType : uint8_t {
  kUnknown = 0,

  kVertex,
  kGeometry,
  kHull,
  kTessellator,
  kDomain,
  kPixel,

  kCompute,

  kRayGen,
  kRayIntersect,
  kRayAnyHit,
  kRayClosestHit,
  kRayMiss,
  kRayCallable,

  kMesh,
  kTask,

  kInvalid = std::numeric_limits<uint8_t>::max()
};

std::vector<std::pair<fs::path, ShaderType>> _shaders;
std::vector<std::pair<fs::path, std::vector<char>>> _blobs;

#include <d3dcompiler.h>


int main() {

	fs::path cwd = fs::current_path();

	for (auto const& dir_entry : fs::recursive_directory_iterator("cwd")) {
    if (dir_entry.path().extension() == "hlsl") {
      ShaderType type;
      if (dir_entry.path().stem().string().ends_with("VS")) {
        type = ShaderType::kVertex;
      } else if (dir_entry.path().stem().string().ends_with("PS")) {
        type = ShaderType::kPixel;
			}

      _shaders.push_back({dir_entry.path(), type});
		}
	}





}
