#pragma once

#include "common.h"

#include "d3d_utility.h"

class D3D11Renderer {
public:
	bool _vsyncEnabled;
	int _vram;
	char _gpuDesciption[128];

	IDXGISwapChain* _swapchain{ nullptr };
	ID3D11Device* _device{ nullptr };
	ID3D11DeviceContext* _deviceContext{ nullptr };
	ID3D11RenderTargetView* _renderTargetView{ nullptr };
	ID3D11Texture2D* _depthStencilBuffer{ nullptr };
	ID3D11DepthStencilState* _depthStencilState{ nullptr };
	ID3D11DepthStencilView* _depthStencilView{ nullptr };
	ID3D11RasterizerState* _raterizerState{ nullptr };

	D3D11Renderer();
	D3D11Renderer(const D3D11Renderer&);
	~D3D11Renderer();

	bool Initialize();
	void Shutdown();

private:

	bool InitD3D();

	XMMATRIX _proj;
	XMMATRIX _orth;
	D3D11_VIEWPORT _viewport;

	bool InitViewport();

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

	bool InitVertices();
};
