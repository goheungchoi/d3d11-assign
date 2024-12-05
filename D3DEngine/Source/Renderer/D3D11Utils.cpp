#include "D3DEngine/Renderer/D3D11Utils.h"

#include <Directxtk/DDSTextureLoader.h>
#include <Directxtk/WICTextureLoader.h>
#include <comdef.h>

#pragma comment(lib, "d3dcompiler.lib")

LPCWSTR GetComErrorString(HRESULT hr) {
  _com_error err(hr);
  LPCWSTR errMsg = err.ErrorMessage();
  return errMsg;
}

std::vector<uint8_t> CompileShaderFromFile(const WCHAR* filename,
                                           LPCSTR entryPoint,
                                           LPCSTR shaderModel) {
  DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifndef NDEBUG
  dwShaderFlags |= D3DCOMPILE_DEBUG;  // Embed debug information in the shaders
  dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;  // Disable optimization
#endif

	std::wprintf(L"Compiling HLSL shader: %ls [%s]\n", filename, entryPoint);

  ComPtr<ID3DBlob> shaderBlob;
  ComPtr<ID3DBlob> errorBlob;
  HRESULT res = D3DCompileFromFile(filename,  // File name
                                   nullptr,   // Shader macro
                                   D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                   entryPoint, shaderModel, dwShaderFlags,
                                   0,  // Shader flags 1, and 2
                                   &shaderBlob, &errorBlob);
  if (FAILED(res)) {
    std::string errorMsg = "Shader compilation failed: " + Utility::convertToUTF8(filename);
    if (errorBlob) {
      errorMsg += std::string("\n") +
                  static_cast<const char*>(errorBlob->GetBufferPointer());
    }
    ErrorExit(Utility::convertToUTF16(errorMsg).data());
  }

  std::vector<uint8_t> data(shaderBlob->GetBufferSize());
  memcpy(data.data(), shaderBlob->GetBufferPointer(), data.size());
  return data;
}

HRESULT CompileShaderFromFile(const WCHAR* filename, LPCSTR entryPoint,
                              LPCSTR shaderModel, ID3DBlob** outShaderBlob,
                              ID3DBlob** outErrorBlob) {
  DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifndef NDEBUG
  dwShaderFlags |= D3DCOMPILE_DEBUG;  // Embed debug information in the shaders
  dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;  // Disable optimization
#endif

  HRESULT res = D3DCompileFromFile(filename,  // File name
                                   nullptr,   // Shader macro
                                   D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                   entryPoint, shaderModel, dwShaderFlags,
                                   0,  // Shader flags 1, and 2
                                   outShaderBlob, outErrorBlob);

  return res;
}

HRESULT ReadBinaryFile(const WCHAR* filename, std::vector<uint8_t>* outData,
                       std::size_t* outSize) {
  std::ifstream file(filename, std::ios::ate | std::ios::binary);

  if (!file.is_open()) {
    return E_INVALIDARG;
  }

  std::size_t filesize = static_cast<std::size_t>(file.tellg());
  // Return file size.
  if (outSize != nullptr) *outSize = filesize;

  if (outData == nullptr) {
    file.close();
    return S_OK;
  }

  std::vector<uint8_t> buffer(filesize);

  file.seekg(0);
  file.read((char*)buffer.data(), filesize);
  file.close();

  *outData = std::move(buffer);

  return S_OK;
}

void ErrorExit(LPCTSTR lpszFunction) {
  // Retrieve the system error message for the last-error code

  LPVOID lpMsgBuf;
  LPVOID lpDisplayBuf;
  DWORD dw = GetLastError();

  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&lpMsgBuf, 0, NULL);

  // Display the error message and exit the process

  lpDisplayBuf = (LPVOID)LocalAlloc(
      LMEM_ZEROINIT,
      (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) *
          sizeof(TCHAR));
  StringCchPrintf((LPTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR),
                  TEXT("%s failed with error %d: %s"), lpszFunction, dw,
                  lpMsgBuf);
  MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

  LocalFree(lpMsgBuf);
  LocalFree(lpDisplayBuf);
  ExitProcess(dw);
}

#if _WIN32
std::string Utility::convertToUTF8(const std::wstring& wstr) {
  const int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1,
                                             nullptr, 0, nullptr, nullptr);
  const std::unique_ptr<char[]> buffer(new char[bufferSize]);
  WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, buffer.get(), bufferSize,
                      nullptr, nullptr);
  return std::string(buffer.get());
}

std::wstring Utility::convertToUTF16(const std::string& str) {
  const int bufferSize =
      MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, nullptr, 0);
  const std::unique_ptr<wchar_t[]> buffer(new wchar_t[bufferSize]);
  MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, buffer.get(), bufferSize);
  return std::wstring(buffer.get());
}
#endif  // _WIN32
