#pragma once

#include "common.h"

#include "d3d_utility.h"

class D3D11Renderer {
private:
	bool _vsyncEnabled{ false };
	int _vram{ 0 };
	char _gpuDesciption[128]{};

	const D3D_FEATURE_LEVEL _minFeatureLevel{ D3D_FEATURE_LEVEL_10_0 };
	D3D_FEATURE_LEVEL _featureLevel{};
	
	D3D11Renderer(const D3D11Renderer&) = delete;
	D3D11Renderer& operator=(const D3D11Renderer&) = delete;

public:

	D3D11Renderer() {}

	HRESULT Initialize(HWND hWnd);
	void Shutdown();

	void BeginDraw();
	void EndDraw();

	Color backgroundColor{ 0.f, 0.f, 0.f, 1.f };
	
	ID3D11Device* _device{ nullptr };
	ID3D11DeviceContext* _deviceContext{ nullptr };
	IDXGIFactory2* _dxgiFactory{ nullptr };
	IDXGISwapChain1* _swapchain{ nullptr };

	HRESULT CreateDeviceResources();
	HRESULT CreateDebugLayer();
	HRESULT CreateFactory();
	HRESULT CreateSwapchain(HWND hWnd);

	ID3D11Texture2D* _backbuffer{ nullptr };
	D3D11_TEXTURE2D_DESC _backbufferDesc{};
	ID3D11RenderTargetView* _renderTargetView{ nullptr };

	ID3D11RasterizerState* _rasterizerState{ nullptr };

	ID3D11Texture2D* _depthStencilBuffer{ nullptr };
	ID3D11DepthStencilState* _depthStencilState{ nullptr };
	ID3D11DepthStencilView* _depthStencilView{ nullptr };

	D3D11_VIEWPORT _viewport{};

	HRESULT ConfigureBackBuffer();
	HRESULT ConfigureDepthStencilBuffer();
	HRESULT ConfigureRasterizer();
	HRESULT ConfigureViewport();
};
