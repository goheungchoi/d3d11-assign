#pragma once

#include "common.h"

#include "D3DEngine/GameEngine/GameEngine.h"

#include "d3d_utility.h"

class DemoApp : public GameEngine
{
	using Super = GameEngine;

#ifndef NDEBUG 
	float frameTime{ 0.f };
	std::size_t fps{ 0 };
	std::size_t count{ 0 };
#endif

public:

	bool isInitialized{ false };

	void Initialize();
	void Execute();
	void Shutdown();

private:
	void FixedUpdate(float) override;
	void Update(float) override;
	void Render() override;

private:

	const D3D_FEATURE_LEVEL _minFeatureLevel{ D3D_FEATURE_LEVEL_10_0 };
	D3D_FEATURE_LEVEL _featureLevel;
	ID3D11Device* _device{ nullptr };
	ID3D11DeviceContext* _deviceContext{ nullptr };
	IDXGIFactory2* _dxgiFactory;
	IDXGISwapChain1* _swapchain{ nullptr };

	HRESULT CreateDeviceResources();
	HRESULT CreateDebugLayer();
	HRESULT CreateFactory();
	HRESULT CreateSwapchain();
	
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
	void InitTransformMatrices();
	void InitCamera();
	void InitModels();

	bool InitD3D();
	void CleanD3D();

	XMMATRIX _proj;
	XMMATRIX _view;
	XMMATRIX _orth;

	HRESULT ReleaseBackBuffer();
	HRESULT ReleaseDepthStencilBuffer();
	HRESULT FlushDeviceContext();

private:

	ID3D11InputLayout* _inputLayout{ nullptr };
	ID3D11VertexShader* _vs{ nullptr };
	ID3D11PixelShader* _ps{ nullptr };

	bool InitPipeline();

private:

	ID3D11Buffer* _vbo{ nullptr };
	UINT _vertexBufferStride{ 0U };
	UINT _vertexBufferOffset{ 0U };
	UINT _vertexCount{ 0U };

	ID3D11Buffer* _ibo{ nullptr };
	UINT _indexBufferStride{ 0U };
	UINT _indexBufferOffset{ 0U };
	UINT _indexCount{ 0U };

	ID3D11Buffer* _cbo{ nullptr };
	ConstantBuffer _cbData{};

	bool InitVertices();
};
