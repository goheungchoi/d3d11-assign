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

#define DECLSPEC_CBUFFER_ALIGN __declspec(align(16))

#define MAX_BONES 100
#define MAX_BONE_WEIGHTS 8

// Vertex struct
struct Vertex {
	Vector3 position;
	Vector2 uv;
	Vector3 normal;
	Vector3 tangent;
	//-----------------------
	int boneIDs[MAX_BONE_WEIGHTS];
	float boneWeights[MAX_BONE_WEIGHTS];

	static constexpr
	Vertex Default() {
		Vertex v{};
		for (int i = 0; i < MAX_BONE_WEIGHTS; ++i) {
			v.boneIDs[i] = -1;
			v.boneWeights[i] = 0.f;
		}
		return v;
	}

	void SetBoneData(int boneID, float weight) {
		for (int i = 0; i < MAX_BONE_WEIGHTS; ++i) {
			// Find the next empty spot
			if (boneIDs[i] < 0) {
				boneIDs[i] = boneID;
				boneWeights[i] = weight;
			}
		}
	}
};

// NOTE: just for an assigment
struct DefaultVertex {
	Vector3 position;
	Vector4 color;
	Vector2 uv;
	Vector3 normal;
	Vector3 tangent;
};

using Index = uint32_t;

struct cbDefaultLightProp {
	Vector4 eyePosition;
	Vector4 lightDir;
	Vector4 lightColor;
};

inline Vector4 g_defaultLightDir{ 0.f, -1.f, 0.f, 1.f };
inline Vector4 g_defaultLightColor{ 1.f, 1.f, 1.f, 1.f };

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
struct _Material {
	Vector4 emissive;
	Vector4 diffuse;
	Vector4 specular;
	Vector4 normal;
	//-----------------------
	float shininess;
	uint32_t useDiffuseTexture;
	float padding[2];
	//-----------------------
	uint32_t useSpecularTexture;
	uint32_t useNormalTexture;
	uint32_t useEmissiveTexture;
	uint32_t useOpacityTexture;
};

inline std::optional<Texture> FindTexture(std::vector<Texture> textures, TEXTURE_TYPE type) {
	auto it = std::find_if(begin(textures), end(textures), 
		[type](const Texture& tex) {
		return tex.type == type;
	});

	return (it != textures.end()) ? std::optional(*it) : std::nullopt;
}

DECLSPEC_CBUFFER_ALIGN
struct cbMaterialProperties {
	_Material material;
};

constexpr uint32_t MAX_LIGHTS = 8;
enum LightType {
	Undefined = 0,
	Directional = 1,
	Point = 2,
	Spot = 3,
	NUM_LIGHT_TYPES
};

struct Light
{
	Vector4 position; // 16 bytes
	//----------------------------------- (16 byte boundary)
	Vector4 direction; // 16 bytes
	//----------------------------------- (16 byte boundary)
	Vector4 color; // 16 bytes
	//----------------------------------- (16 byte boundary)
	float spotAngle; // 4 bytes
	float constAtt; // 4 bytes
	float linearAtt; // 4 bytes
	float quadAtt; // 4 bytes
	//----------------------------------- (16 byte boundary)
	uint32_t lightType; // 4 bytes
	uint32_t enabled; // 4 bytes
	float padding[2]; // 8 bytes
	//----------------------------------- (16 byte boundary)
}; // Total: 80

DECLSPEC_CBUFFER_ALIGN
struct cbLightProperties {
	Vector4 eyePosition;
	Vector4 globalAmbient;
	Light lights[MAX_LIGHTS];
};

inline cbLightProperties g_lightProperties;

inline 
void SetGlobalEyePosition(const Vector4& eyePosition) {
	g_lightProperties.eyePosition = eyePosition;
}

inline
void SetGlobalAmbient(const Vector4& globalAmbient) {
	g_lightProperties.globalAmbient = globalAmbient;
}

inline int __curr_light_index{ -1 };

inline
void PushBackLight(const Light* light) {
	if (__curr_light_index >= int(MAX_LIGHTS) - 1) return;
	g_lightProperties.lights[++__curr_light_index] = *light;
}

inline
void PopBackLight() {
	if (__curr_light_index < 0) return;
	g_lightProperties.lights[__curr_light_index--] = {};
}

// MVP Transform Constant buffer
struct cbPerFrame {
	Matrix viewProj;
};

struct cbPerObject {
	Matrix model;
	Matrix inverseTransposeModel;
	Matrix modelViewProj;
	//-----------------------
	Matrix boneTransforms[MAX_BONES];	// 64 x 100 = 6400 bytes
};

// Camera properties
inline Vector4 g_camPos{ 0.f, 100.f, -200.f , 0.f };

inline cbPerFrame g_cbPerFrame{};

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

// Utility function to read the data and size of a binary file
HRESULT ReadBinaryFile(const WCHAR* filename, std::vector<uint8_t>* data, std::size_t* size);

// Utility function to load textures from a file
HRESULT CreateTextureFromFile(ID3D11Device* device, const wchar_t* filename, ID3D11ShaderResourceView** outTextureView);

// Utility macro to convert D3D API failures into exceptions
#define CHECK(res) if (FAILED(res)) throw COMException(res);

