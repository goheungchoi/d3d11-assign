#pragma once

#include "Renderer/Internal/D3D11Common.h"

#include "Renderer/Internal/D3D11Types.h"

#include "Renderer/Internal/Components/FrameBuffer.h"

namespace DX {

class RenderDevice {
  static constexpr UINT kMaxSample{16};

  D3D_FEATURE_LEVEL _d3dFeatureLevel{D3D_FEATURE_LEVEL_11_0};
  int _debugAdapterOrdinal{-1};

  ComPtr<IDXGIFactory2> _dxgiFactory;
  ComPtr<IDXGIAdapter1> _dxgiAdapter;
  ComPtr<ID3D11Device1> _d3dDevice;
  ComPtr<ID3D11DeviceContext1> _d3dImmContext;

  RenderDevice() = default;

 public:
  static RenderDevice* CreateRenderDevice() {
    RenderDevice* device = new RenderDevice();
    device->Initialize();
    return device;
  }

	ID3D11Device1* GetDevice() { return _d3dDevice.Get(); }
  IDXGIFactory2* GetDXGIFactory() { return _dxgiFactory.Get(); }
  IDXGIAdapter1* GetAdapter() { return _dxgiAdapter.Get(); }
  ID3D11DeviceContext* GetImmediateContext() { return _d3dImmContext.Get(); }

	void GetMultisampleCountAndQualityLevels(DXGI_FORMAT format, UINT* outSampleCount, UINT* outQuality) {
    UINT quality{0};
    UINT samples = kMaxSample;

		for (; samples > 1; samples /= 2) {
      _d3dDevice->CheckMultisampleQualityLevels(format, samples, &quality);
      if (quality > 0) break;
		}

		if (quality > 0) {
      *outSampleCount = samples;
      *outQuality = quality;
    } else {
      *outSampleCount = 0;
      *outQuality = 0;
		}
	}

	UINT GetMultisampleMaxCount(DXGI_FORMAT format) {
    UINT quality{0};
    UINT samples = kMaxSample;

    for (; samples > 1; samples /= 2) {
      _d3dDevice->CheckMultisampleQualityLevels(format, samples, &quality);
      if (quality > 0) break;
    }

    return samples;
	}

	UINT GetMultisampleQualityLevels(DXGI_FORMAT format, UINT sampleCount) {
    UINT quality{0};
    _d3dDevice->CheckMultisampleQualityLevels(format, sampleCount, &quality);
    return quality;
	}

  ComPtr<ID3D11Buffer> CreateConstantBuffer(D3D11_USAGE usage, const void* data,
                                            UINT size) const;
	template <typename T>
  ComPtr<ID3D11Buffer> CreateConstantBuffer(
      D3D11_USAGE usage = D3D11_USAGE_DYNAMIC, const T* data = nullptr) const {
    static_assert(sizeof(T) == Utility::roundToPowerOfTwo(sizeof(T), 16));
    return CreateConstantBuffer(usage, data, sizeof(T));
  }

	void CopyMappedData(ComPtr<ID3D11Buffer>& buffer, const void* data, UINT size);
  template <typename T>
  void CopyMappedData(ComPtr<ID3D11Buffer>& buffer, const T* data) {
    static_assert(sizeof(T) == Utility::roundToPowerOfTwo(sizeof(T), 16));
    CopyMappedData(buffer, data, sizeof(T));
  }

  void CopyData(ComPtr<ID3D11Buffer>& buffer, const void* data);

	MeshBuffer CreateMeshBuffer(const MeshData& data);

  TextureBuffer CreateTextureBuffer(UINT width, UINT height, DXGI_FORMAT format,
                                    D3D11_BIND_FLAG flags, UINT mipLevels = 1);
  TextureBuffer CreateTextureBuffer(void* data, UINT pixelByteSize, UINT width, UINT height,
                                    DXGI_FORMAT format, D3D11_BIND_FLAG flags,
                                    UINT mipLevels = 1);
  TextureBuffer CreateTextureBuffer(const TextureData& data,
                                    D3D11_BIND_FLAG flags = (D3D11_BIND_FLAG)0);

	CubeTextureBuffer CreateCubeTextureBuffer(UINT width, UINT height,
                                            DXGI_FORMAT format,
                                            D3D11_BIND_FLAG flags,
                                            UINT mipLevels = 1, UINT arrayLayers = 6);
  CubeTextureBuffer CreateCubeTextureBuffer(const TextureData& data,
                                            D3D11_BIND_FLAG flags);

	RenderTargetBuffer CreateRenderTargetBuffer(UINT width, UINT height, DXGI_FORMAT format,
                                 UINT samples = 1);
  DepthStensilBuffer CreateDepthStencilBuffer(UINT width, UINT height,
                                              DXGI_FORMAT format,
                                              UINT samples = 1);

	FrameBuffer* CreateFrameBuffer(
      UINT width, UINT height, UINT samples,
      std::initializer_list<RenderTargetBuffer> colorAttachments,
      std::optional<DepthStensilBuffer> depthAttachment = std::nullopt);

	ComPtr<ID3D11SamplerState> CreateSamplerState(
      D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode) const;

	class SwapChain* CreateSwapChain(HWND hwnd, UINT width, UINT height, bool allowTearing);

	class RenderContext* CreateRenderContext();
	
	// TODO: Submit render context;
  void SubmitCommandList(class RenderContext* deferredContext);

 private:
  bool CheckSdkLayersSupport() noexcept {
    HRESULT hr = D3D11CreateDevice(
        nullptr, D3D_DRIVER_TYPE_NULL, nullptr, D3D11_CREATE_DEVICE_DEBUG,
        nullptr, 0, D3D11_SDK_VERSION, nullptr, nullptr, nullptr);
    return SUCCEEDED(hr);
  }

	bool CheckAllowTearingSupport() {
    BOOL allowTearing{FALSE};

    ComPtr<IDXGIFactory5> factory5;
    HRESULT hr = _dxgiFactory.As(&factory5);
    if (SUCCEEDED(hr)) {
      hr = factory5->CheckFeatureSupport(DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                                         &allowTearing, sizeof(allowTearing));
    }

		return allowTearing;
	}

	bool CheckHDRSupport() {
    ComPtr<IDXGIFactory4> factory4;
    if (FAILED(_dxgiFactory.As(&factory4))) {
      return false;
		}
    return true;
	}

	bool CheckFlipPresent() {
    ComPtr<IDXGIFactory4> factory4;
    if (FAILED(_dxgiFactory.As(&factory4))) {
      return false;
		}
    return true;
	}

  void CreateFactory() {
#ifdef _DEBUG
    bool debugDXGI{false};
    {
      ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
      if (SUCCEEDED(DXGIGetDebugInterface1(
              0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf())))) {
        debugDXGI = true;

        ThrowIfFailed(CreateDXGIFactory2(
            DXGI_CREATE_FACTORY_DEBUG,
            IID_PPV_ARGS(_dxgiFactory.ReleaseAndGetAddressOf())));

        dxgiInfoQueue->SetBreakOnSeverity(
            DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
        dxgiInfoQueue->SetBreakOnSeverity(
            DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

        DXGI_INFO_QUEUE_MESSAGE_ID hide[] = {
            80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter
                  does not control the output on which the swapchain's window
                  resides. */
            ,
        };
        DXGI_INFO_QUEUE_FILTER filter = {};
        filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
        filter.DenyList.pIDList = hide;
        dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
      }
    }

    if (!debugDXGI)
#endif
      ThrowIfFailed(CreateDXGIFactory1(
          IID_PPV_ARGS(_dxgiFactory.ReleaseAndGetAddressOf())));
  }

	void GetHardwareAdapter() { 
		ComPtr<IDXGIAdapter1> adapter;

		ComPtr<IDXGIFactory6> factory6;
    HRESULT hr = _dxgiFactory.As(&factory6);
    if (SUCCEEDED(hr)) {
      for (int adapterIndex = 0; SUCCEEDED(factory6->EnumAdapterByGpuPreference(
               adapterIndex, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
               IID_PPV_ARGS(adapter.ReleaseAndGetAddressOf())));
           adapterIndex++) {
        DXGI_ADAPTER_DESC1 desc;
        ThrowIfFailed(adapter->GetDesc1(&desc));

				if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
          continue;
				}
#ifdef _DEBUG
        wchar_t buff[256] = {};
        swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n",
                   adapterIndex, desc.VendorId, desc.DeviceId,
                   desc.Description);
        OutputDebugString(buff);
#endif

				break;
      }
    }

		if (!adapter) {
      for (UINT adapterIndex = 0; SUCCEEDED(_dxgiFactory->EnumAdapters1(
               adapterIndex, adapter.ReleaseAndGetAddressOf()));
           adapterIndex++) {
        DXGI_ADAPTER_DESC1 desc;
        ThrowIfFailed(adapter->GetDesc1(&desc));

        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
          // Don't select the Basic Render Driver adapter.
          continue;
        }

        if (_debugAdapterOrdinal == -1 ||
            (_debugAdapterOrdinal == int(adapterIndex))) {
#ifdef _DEBUG
          wchar_t buff[256] = {};
          swprintf_s(buff, L"Direct3D Adapter (%u): VID:%04X, PID:%04X - %ls\n",
                     adapterIndex, desc.VendorId, desc.DeviceId,
                     desc.Description);
          OutputDebugStringW(buff);
#endif
          break;
        }
      }
		}

		_dxgiAdapter = adapter;
    if (!_dxgiAdapter) {
      throw std::runtime_error("No Direct3D hardware device found!");
		}
	}

	void CreateDevice() {
    UINT creationFlags{};

#ifdef _DEBUG
    if (CheckSdkLayersSupport()) {
      creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
    } else {
      OutputDebugString(L"WARNING: Direct3D Debug Device is not available!\n");
    }
#endif

		if (!CheckAllowTearingSupport()) {
      OutputDebugString(L"WARNING: Allow tearing is not supported!");
		}

		if (!CheckHDRSupport()) {
      OutputDebugString(L"WARNING: HDR swapchain is not supported!");
		}

		if (!CheckFlipPresent()) {
      OutputDebugString(L"WARNING: Flip mode is not supported!");
		}

		static const D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
    };

		// Create the Direct3D 11 API device object and a corresponding context.
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
		
		HRESULT hr = E_FAIL;
    if (_dxgiAdapter) {
      hr = D3D11CreateDevice(
          _dxgiAdapter.Get(), D3D_DRIVER_TYPE_UNKNOWN, nullptr, creationFlags,
          featureLevels, std::size(featureLevels), D3D11_SDK_VERSION,
          device.GetAddressOf(), &_d3dFeatureLevel, context.GetAddressOf());
    }
#ifdef _DEBUG
		else {
      throw std::runtime_error("No Direct3D hardware device found!");
		}
#endif 

		if (FAILED(hr)) {
      // If the initialization fails, fall back to the WARP device.
      // For more information on WARP, see:
      // http://go.microsoft.com/fwlink/?LinkId=286690
      hr = D3D11CreateDevice(
          nullptr,
          D3D_DRIVER_TYPE_WARP,  // Create a WARP device instead of a hardware
                                 // device.
          nullptr, creationFlags, featureLevels, std::size(featureLevels),
          D3D11_SDK_VERSION, device.GetAddressOf(), &_d3dFeatureLevel,
          context.GetAddressOf());

      if (SUCCEEDED(hr)) {
        OutputDebugString(L"Direct3D Adapter - WARP\n");
      }
    }

		ThrowIfFailed(hr);

		// Debug info queue
#ifdef _DEBUG
    ComPtr<ID3D11Debug> d3dDebug;
    if (SUCCEEDED(device.As(&d3dDebug))) {
      ComPtr<ID3D11InfoQueue> d3dInfoQueue;
      if (SUCCEEDED(d3dDebug.As(&d3dInfoQueue))) {

        d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION,
                                         true);
        d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
        D3D11_MESSAGE_ID hide[] = {
            D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
        };
        D3D11_INFO_QUEUE_FILTER filter = {};
        filter.DenyList.NumIDs = static_cast<UINT>(std::size(hide));
        filter.DenyList.pIDList = hide;
        d3dInfoQueue->AddStorageFilterEntries(&filter);
      }
    }
#endif

		ThrowIfFailed(device.As(&_d3dDevice));
    ThrowIfFailed(context.As(&_d3dImmContext));
	}

  void Initialize() {

		// Create dxgi factory
    CreateFactory();

		// Get hardware adapter
    GetHardwareAdapter();

		// Create the logical device
    CreateDevice();
  }
};

}  // namespace DX