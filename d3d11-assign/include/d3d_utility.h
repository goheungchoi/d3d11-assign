#pragma once

#include "common.h"

#include <dxgi1_5.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>

#ifdef _DEBUG
#include <dxgidebug.h>
#endif

#include <wrl/client.h>	// ComPtr
using Microsoft::WRL::ComPtr;

#include <directxtk/SimpleMath.h>
using namespace DirectX;
using namespace DirectX::SimpleMath;

// Vertex struct
struct Vertex {
	Vector3 position;
	Vector3 normal;	// TODO: Update color -> normal
	Vector2 uv;
};

using Index = uint32_t;

// Transform struct
struct Transform {
	// Local space transform
	Vector3 scale;
	Vector3 rotate;	// Euler rotation { pitch, yaw, roll }
	Vector3 translate;
};

// Texture Types
enum class TEXTURE_TYPE {
	NONE,
	DIFFUSE,
	SPECULAR,
	AMBIENT,
	EMISSIVE,
	NORMALS,
	HEIGHT,
	OPACITY,
	DISPLACEMENT,
	LIGHTMAP,
	REFLECTION,

	/* PBR Materials */
	ALBEDO,
	METALICNESS,
	DIFFUSE_ROUGHNESS,
	AMBIENT_OCCLUSION,
	EMISSIVE_COLOR,
	NORMAL_CAMERA,
};

// Texture struct
struct Texture {
	ID3D11ShaderResourceView* textureView;
	ID3D11SamplerState* samplerState;
	TEXTURE_TYPE type;
	std::string path;	// Texture equality check; TODO: need optimization
};

// Material
struct Material {
	Texture diffuse;
	Texture specular;
	Texture normal;
	float shininess;
};

// MVP Transform Constant buffer
struct cbPerFrame {
	Matrix viewProj;
};

struct cbPerObject {
	Matrix model;
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
HRESULT CompileShaderFromFile(const WCHAR* filename, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** outShaderBlob, ID3DBlob** outErrorBlob = nullptr);

// Utility function to load textures from a file
HRESULT CreateTextureFromFile(ID3D11Device* d3dDevice, const wchar_t* szFileName, ID3D11ShaderResourceView** outTextureView);

// Utility macro to convert D3D API failures into exceptions
#define CHECK(res) if (FAILED(res)) throw COMException(res);

