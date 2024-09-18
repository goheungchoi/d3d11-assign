#include "d3d_utility.h"

#include <comdef.h>

#include <Directxtk/DDSTextureLoader.h>
#include <Directxtk/WICTextureLoader.h>

#pragma comment(lib, "d3dcompiler.lib")

LPCWSTR GetComErrorString(HRESULT hr)
{
	_com_error err(hr);
	LPCWSTR errMsg = err.ErrorMessage();
	return errMsg;
}

HRESULT CompileShaderFromFile(
	const WCHAR* filename, 
	LPCSTR entryPoint, 
	LPCSTR shaderModel, 
	ID3DBlob** outShaderBlob,
	ID3DBlob** outErrorBlob) {

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifndef NDEBUG
	dwShaderFlags |= D3DCOMPILE_DEBUG;	// Embed debug information in the shaders
	dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;	// Disable optimization
#endif

	HRESULT res = D3DCompileFromFile(
		filename,					// File name
		nullptr,					// Shader macro
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entryPoint,
		shaderModel,
		dwShaderFlags, 0,	// Shader flags 1, and 2
		outShaderBlob,
		outErrorBlob
	);

	return res;
}

HRESULT ReadBinaryFile(const WCHAR* filename, std::vector<uint8_t>* outData, std::size_t* outSize)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		return E_INVALIDARG;
	}

	std::size_t filesize = static_cast<std::size_t>(file.tellg());
	// Return file size.
	*outSize = filesize;

	if (outData == nullptr) 
		return S_OK;

	std::vector<uint8_t> buffer(filesize);

	file.seekg(0);
	file.read((char*)buffer.data(), filesize);
	file.close();

	*outData = std::move(buffer);

	return S_OK;
}

HRESULT CreateTextureFromFile(ID3D11Device* d3dDevice, const wchar_t* szFileName, ID3D11ShaderResourceView** outTextureView)
{
	return E_NOTIMPL;
}
