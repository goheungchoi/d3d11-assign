#include "d3d_renderer.h"

HRESULT D3D11Renderer::Initialize(HWND hWnd)
{
	HRESULT res = S_OK;

	res = CreateDeviceResources();
	if (FAILED(res)) return res;

	res = CreateDebugLayer();
	if (FAILED(res)) return res;

	res = CreateFactory();
	if (FAILED(res)) return res;

	res = CreateSwapchain(hWnd);
	if (FAILED(res)) return res;

	res = ConfigureBackBuffer();
	if (FAILED(res)) return res;

	res = ConfigureDepthStencilBuffer();
	if (FAILED(res)) return res;

	res = ConfigureRasterizer();
	if (FAILED(res)) return res;

	res = ConfigureBlendState();
	if (FAILED(res)) return res;

	res = ConfigureViewport();
	if (FAILED(res)) return res;

	return res;
}

void D3D11Renderer::Shutdown()
{
}

void D3D11Renderer::BeginDraw()
{
	// Clear the render target
	_deviceContext->ClearRenderTargetView(
		_renderTargetView, 
		backgroundColor
	);
	// Clear the z-buffer
	_deviceContext->ClearDepthStencilView(
		_depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		// Depth clear value		// Stencil clear value
		1.f											, 0
	);

	//"fine-tune" the blending equation
	//float blendFactor[] = { 0.75f, 0.75f, 0.75f, 1.0f };

	
	/* OUTPUT MERGER */
	// Bind the render target every frame
	_deviceContext->OMSetRenderTargets(1, &_renderTargetView, _depthStencilView);
	//Set the default blend state (no blending) for opaque objects
	//_deviceContext->OMSetBlendState(_blendState, blendFactor, 0xffffffff);

	//// Set the depth stencil state
	//_deviceContext->OMSetDepthStencilState(_depthStencilState, 1);


	///* RASTERIZATION STAGE */
	//_deviceContext->RSSetState(_rasterizerState);
	//_deviceContext->RSSetViewports(1, &_viewport);
}

void D3D11Renderer::EndDraw()
{
	// Present the render target
	_swapchain->Present(1, 0);
}

HRESULT D3D11Renderer::CreateDeviceResources()
{
	HRESULT res = S_OK;

	// Determin DirectX hardware feature levels this app will support
	static const D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};

	UINT featureLevelCount = 0;
	for (; featureLevelCount < std::size(featureLevels); ++featureLevelCount) {
		if (featureLevels[featureLevelCount] < _minFeatureLevel) break;
	}

	if (featureLevelCount == 0)
		throw std::out_of_range("minFeatureLevel too high!");

	UINT deviceFlags{ 0 };
#ifndef NDEBUG
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	res = D3D11CreateDevice(
		nullptr,		// Use the default adapter
		D3D_DRIVER_TYPE_HARDWARE,
		0,
		deviceFlags,
		featureLevels,
		std::size(featureLevels),
		D3D11_SDK_VERSION,
		&_device,
		&_featureLevel,
		&_deviceContext
	);

	if (FAILED(res)) return res;

	return res;
}

HRESULT D3D11Renderer::CreateDebugLayer()
{
	ID3D11Debug* d3dDebug = nullptr;
	if (SUCCEEDED(_device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug)))
	{
		ID3D11InfoQueue* d3dInfoQueue = nullptr;
		if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)))
		{
#ifdef _DEBUG
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif

			D3D11_MESSAGE_ID hide[] =
			{
				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
				// Add more message IDs here as needed
			};

			D3D11_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			d3dInfoQueue->AddStorageFilterEntries(&filter);
			d3dInfoQueue->Release();

			return S_OK;
		}
		d3dDebug->Release();
	}

	return S_FALSE;
}

HRESULT D3D11Renderer::CreateFactory()
{
#ifdef _DEBUG
	bool debugDXGI = false;
	{
		ComPtr<IDXGIInfoQueue> dxgiInfoQueue;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(dxgiInfoQueue.GetAddressOf()))))
		{
			debugDXGI = true;

			CHECK(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory)));

			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
			dxgiInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

			DXGI_INFO_QUEUE_MESSAGE_ID hide[] =
			{
					80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
			};
			DXGI_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			dxgiInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
		}
	}

	if (!debugDXGI)
#endif
		CHECK(CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory)));

	return S_OK;
}

HRESULT D3D11Renderer::CreateSwapchain(HWND hWnd)
{
	HRESULT res = S_OK;

	DXGI_SWAP_CHAIN_DESC1 desc{
		.Width = SCREEN_WIDTH,
		.Height = SCREEN_HEIGHT,
		// Swap buffer description
		.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
		// Sample description
		.SampleDesc = {
			.Count = 1,	// No multisampling
			.Quality = 0,
		},
		// Swap buffer usage
		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
		// The number of buffers to use
		.BufferCount = 2,
		.Scaling = DXGI_SCALING_NONE,
		// Swap effect
		.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL,
		// Alpha mode
		.AlphaMode = DXGI_ALPHA_MODE_IGNORE,
		//// Allow full screen switching
		//.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
	};

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullScreenDesc = {};
	fullScreenDesc.Windowed = TRUE;

	// Create a swapchain
	ComPtr<IDXGIAdapter> adapter;
	res = _dxgiFactory->EnumAdapters(0, &adapter);
	if (FAILED(res)) return res;

	res = _dxgiFactory->CreateSwapChainForHwnd(
		_device,
		hWnd,
		&desc,
		&fullScreenDesc,	// A windowed swapchain. For a full-screen swapchain, configure  DXGI_SWAP_CHAIN_FULLSCREEN_DESC
		NULL,	// No restrict to output
		&_swapchain
	);

	return res;
}

HRESULT D3D11Renderer::ConfigureBackBuffer()
{
	HRESULT res = S_OK;

	// Create a back buffer
	res = _swapchain->GetBuffer(
		0,
		__uuidof(ID3D11Texture2D),
		(void**)&_backbuffer
	);
	if (FAILED(res)) return res;

	// Create a render target view
	res = _device->CreateRenderTargetView(
		_backbuffer,
		nullptr,
		&_renderTargetView
	);
	if (FAILED(res)) return res;

	// Get the description
	_backbuffer->GetDesc(&_backbufferDesc);

	return res;
}

HRESULT D3D11Renderer::ConfigureDepthStencilBuffer()
{
	HRESULT res = S_OK;

	/* Depth Stencil Buffer */

	// Create the depth stencil buffer
	D3D11_TEXTURE2D_DESC depthBufferDesc{};
	depthBufferDesc.Width = _backbufferDesc.Width;
	depthBufferDesc.Height = _backbufferDesc.Height;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;	// No multisampling
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.CPUAccessFlags = 0;	// No CPU r/w access
	depthBufferDesc.MiscFlags = 0;	// Optional flags none
	res = _device->CreateTexture2D(
		&depthBufferDesc,
		nullptr,
		&_depthStencilBuffer
	);
	if (FAILED(res)) return res;

	/* Depth Stencil State */

	// Depth stencil state description
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc{};
	// Set up the depth state
	depthStencilDesc.DepthEnable = true;
	// Turn on writes to the depth-stencil buffer
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	// Set up the stencil state
	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;	// Always pass the comparison

	// Stencil operations if pixel is back-facing
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;	// Always pass the comparison

	// Create the depth stencil state
	_device->CreateDepthStencilState(&depthStencilDesc, &_depthStencilState);

	// Set depth stencil state
	_deviceContext->OMSetDepthStencilState(_depthStencilState, 1);

	/* Depth Stencil View */

	// Depth stencil 
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view
	_device->CreateDepthStencilView(
		_depthStencilBuffer,
		&depthStencilViewDesc,
		&_depthStencilView
	);

	return res;
}

HRESULT D3D11Renderer::ConfigureRasterizer()
{
	HRESULT res = S_OK;

	// Rasteization state description
	D3D11_RASTERIZER_DESC rasterDesc{};
	rasterDesc.FillMode = D3D11_FILL_SOLID;	// Wireframe or solid
	rasterDesc.CullMode = D3D11_CULL_BACK;	// Back-face culling
	// rasterDesc.FrontCounterClockwise = FALSE;	// Front-face is clockwise
	rasterDesc.DepthBias = 0;	// Depth value added to a given pixel
	rasterDesc.DepthBiasClamp = 0.f;	// Max depth bias of a pixel
	rasterDesc.SlopeScaledDepthBias = 0.f;

	rasterDesc.DepthClipEnable = TRUE;	// Enable clipping based on distance
	rasterDesc.ScissorEnable = FALSE;	// Enable scissor-rectangle culling

	rasterDesc.MultisampleEnable = FALSE;
	rasterDesc.AntialiasedLineEnable = FALSE;

	// Create the rasterizer state
	_device->CreateRasterizerState(&rasterDesc, &_rasterizerState);

	// Set the rasterizer state
	_deviceContext->RSSetState(_rasterizerState);

	return res;
}

HRESULT D3D11Renderer::ConfigureBlendState()
{
	HRESULT res = S_OK;

	D3D11_BLEND_DESC blendDesc{};
	D3D11_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc{};

	renderTargetBlendDesc.BlendEnable = true;
	renderTargetBlendDesc.SrcBlend = D3D11_BLEND_SRC_COLOR;
	renderTargetBlendDesc.DestBlend = D3D11_BLEND_BLEND_FACTOR;
	renderTargetBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	renderTargetBlendDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
	renderTargetBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

	blendDesc.AlphaToCoverageEnable = false;
	blendDesc.RenderTarget[0] = renderTargetBlendDesc;

	res = _device->CreateBlendState(&blendDesc, &_blendState);

	return res;
}

HRESULT D3D11Renderer::ConfigureViewport()
{
	HRESULT res = S_OK;

	// Set the viewport
	_viewport = {};
	_viewport.Width = (float)_backbufferDesc.Width;
	_viewport.Height = (float)_backbufferDesc.Height;
	_viewport.MaxDepth = 1.f;
	_viewport.MinDepth = 0.f;
	_deviceContext->RSSetViewports(1, &_viewport);

	return res;
}


