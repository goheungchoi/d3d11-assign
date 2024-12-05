#pragma once

#include "D3DEngine/EngineCommon.h"

#include <d3dcompiler.h>

#include <strsafe.h>

#include <directxtk/SimpleMath.h>
using namespace DirectX;
using namespace DirectX::SimpleMath;

struct Vertex {
	Vector3 position;
	Vector3 normal;
	Vector3 tangent;
	Vector3 bitangent;
	Vector2 texcoord;
	Vector4 color;
};

using Index = uint32_t;

struct Face {
	uint32_t v1, v2, v3;
};

// Utility class for COM exception
class COMException : public std::exception {
	HRESULT res;

public:
	COMException(HRESULT hr) : res{ hr } {}

	const char* what() const noexcept override
	{
		static char s_str[64] = {};
		sprintf_s(s_str, "Failure with HRESULT of %08X",
			static_cast<unsigned int>(res));
		return s_str;
	}
};

LPCWSTR GetComErrorString(HRESULT hr);

// Utility function to compile shaders with D3DCompile
std::vector<uint8_t> CompileShaderFromFile(const WCHAR* filename, LPCSTR entryPoint, LPCSTR shaderModel);

// Utility function to read the data and size of a binary file
HRESULT ReadBinaryFile(const WCHAR* filename, std::vector<uint8_t>* data, std::size_t* size = nullptr);

// Utility macro to convert D3D API failures into exceptions
#define CHECK(res) if (FAILED(res)) throw COMException(res);

class Utility {
 public:
  template <typename T>
  static constexpr bool isPowerOfTwo(T value) {
    return value != 0 && (value & (value - 1)) == 0;
  }
  template <typename T>
  static constexpr T roundToPowerOfTwo(T value, int POT) {
    return (value + POT - 1) & -POT;
  }
  template <typename T>
  static constexpr T numMipmapLevels(T width, T height) {
    T levels = 1;
    while ((width | height) >> levels) {
      ++levels;
    }
    return levels;
  }

#if _WIN32
  static std::string convertToUTF8(const std::wstring& wstr);
  static std::wstring convertToUTF16(const std::string& str);
#endif  // _WIN32
};

void ErrorExit(LPCTSTR lpszFunction);
