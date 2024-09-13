#include "demo_app.h"

#include "D3DEngine/WinApp/WinApp.h"

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "dxguid.lib")

#define USE_FLIPMODE 1	// In order to not show warnings, use flip mode
#define VSYNC_ENABLED 0 // diable v-sync when 0, otherwise v-sync is on

DemoApp* loadedApp{ nullptr };

void DemoApp::Initialize() {
	if (loadedApp) abort();

	///////////////////////// DO NOT MODIFY /////////////////////////
	// 윈도우 생성
	WinApp::App_Init();
	// TODO: 윈도우 타이틀이랑 스타일 추가할 것.
	WindowStyleFlags styleFlags = WS_OVERLAPPED;
	hWindow = WinApp::App_CreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, TITLE, styleFlags);

	// GameEngine의 이니셜라이제이션
	Super::Initialize();
	/////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////////////////////
	// Other initialization stages
	//
	//
	// 

	if (!InitD3D()) return;
	if (!InitPipeline()) return;
	if (!InitVertices()) return;

	// 초기화 True
	isInitialized = true;
	loadedApp = this;
}

void DemoApp::Execute() {
	// 게임 루프 실행
	Super::Execute();
}

void DemoApp::Shutdown() {
	if (isInitialized) {
		/////////////////////////////////////////////////////////////////
		// Other finalization stages
		//
		// 
		// 

		///////////////////////// DO NOT MODIFY /////////////////////////
		// 게임 엔진 셧다운
		Super::Shutdown();

		// 윈도우 파괴
		WinApp::App_Destroy();
		/////////////////////////////////////////////////////////////////
	}

	loadedApp = nullptr;
}


void DemoApp::FixedUpdate(float dt) {


}

void DemoApp::Update(float dt) {
#ifndef NDEBUG 
	frameTime += dt;
#endif
	

}

void DemoApp::Render() {
#ifndef NDEBUG 
	// FPS
	if (frameTime >= 1.0f) {
		frameTime -= 1.0f;
		fps = count;
		count = 0;
	}
#endif

	// Update the contant buffer data
	/*_deviceContext->UpdateSubresource(
		_cbo, 0, nullptr, &_cbData, 0, 0
	);*/

	// Clear the render target
	_deviceContext->ClearRenderTargetView(
		_renderTargetView, 
		Color{ 0.f, 0.5f, 0.5f, 1.f }
	);
	// Clear the z-buffer
	_deviceContext->ClearDepthStencilView(
		_depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		// Depth clear value		// Stencil clear value
		1.f											, 0
	);

	/* OUTPUT MERGER */
	// Bind the render target every frame
	_deviceContext->OMSetRenderTargets(1, &_renderTargetView, _depthStencilView);
	// Set the depth stencil state
	_deviceContext->OMSetDepthStencilState(_depthStencilState, 1);

	/* INPUT ASSEMBLER STAGE */
	_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	_deviceContext->IASetVertexBuffers(0, 1, &_vbo, &_vertexBufferStride, &_vertexBufferOffset);
	_deviceContext->IASetIndexBuffer(_ibo, DXGI_FORMAT_R16_UINT, _indexBufferOffset);
	_deviceContext->IASetInputLayout(_inputLayout);

	/* VERTEX STAGE */
	_deviceContext->VSSetShader(_vs, nullptr, 0);
	// _deviceContext->VSSetConstantBuffers(0, 1, &_cbo);

	/* RASTERIZATION STAGE */
	_deviceContext->RSSetState(_rasterizerState);
	_deviceContext->RSSetViewports(1, &_viewport);

	/* PIXEL STAGE */
	_deviceContext->PSSetShader(_ps, nullptr, 0);

	// Start sending commands to the gpu.
	_deviceContext->DrawIndexed(_indexCount, 0, 0);

	// Present the render target
	_swapchain->Present(1, 0);

#ifndef NDEBUG 
	++count;
#endif
}

HRESULT DemoApp::CreateDeviceResources()
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

HRESULT DemoApp::CreateDebugLayer()
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

HRESULT DemoApp::CreateFactory()
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

HRESULT DemoApp::CreateSwapchain()
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
		hWindow,
		&desc,
		&fullScreenDesc,	// A windowed swapchain. For a full-screen swapchain, configure  DXGI_SWAP_CHAIN_FULLSCREEN_DESC
		NULL,	// No restrict to output
		&_swapchain
	);

	return res;
}

HRESULT DemoApp::ConfigureBackBuffer()
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

HRESULT DemoApp::ConfigureDepthStencilBuffer()
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
	_device->CreateTexture2D(
		&depthBufferDesc,
		nullptr,
		&_depthStencilBuffer
	);

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

HRESULT DemoApp::ConfigureRasterizer()
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

	return res;
}

HRESULT DemoApp::ConfigureViewport()
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

void DemoApp::InitTransformMatrices()
{
	// Setup the vertical field of view
	float vfov = PI_F / 2.f;	// 90 degree field of view
	// Get the screen aspect ratio
	float aspectRatio = (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT;

	// Create the projection matrix
	_proj = XMMatrixPerspectiveFovLH(vfov, aspectRatio, 0.01f, 100.f);
	// Create the Orthographic projection matrix
	_orth = XMMatrixOrthographicLH((float)SCREEN_WIDTH, (float)SCREEN_HEIGHT, 0.01f, 100.f);
}

void DemoApp::InitCamera()
{
	// TODO:
}

void DemoApp::InitModels()
{
	// TODO:
}

//bool DemoApp::InitD3D()
//{
//	HRESULT res{ 0 };
//
//	// Create a swapchain description
//	DXGI_SWAP_CHAIN_DESC swapchainDescription{
//		// Swap buffer description
//		.BufferDesc = {
//			.Width = SCREEN_WIDTH,
//			.Height = SCREEN_HEIGHT,
//			.RefreshRate = {.Numerator = 60, .Denominator = 1 },
//			.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
//		},
//		// Sample description
//		.SampleDesc = {
//			.Count = 1,	// No multisampling
//			.Quality = 0,
//		},
//		// Swap buffer usage
//		.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
//		// The number of buffers to use
//		.BufferCount = 1,
//		// Window handle
//		.OutputWindow = hWindow,
//		// Set window mode
//		.Windowed = true,	// True - window mode, False - full screen
//		// Swap effect
//		.SwapEffect = DXGI_SWAP_EFFECT_DISCARD,
//		// Allow full screen switching
//		.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH
//	};
//
//	// Create a device and swapchain
//	UINT creationFlags{ 0 }; 
//#ifndef NDEBUG	// Enable D3D debug layer
//	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
//#endif
//
//	CHECK(
//		D3D11CreateDeviceAndSwapChain(
//			NULL,											// DXGI Adaptor
//			D3D_DRIVER_TYPE_HARDWARE,	// Driver type
//			NULL,											// Software. Not used unless D3D_DRIVER_TYPE_SOFTWARE
//			creationFlags,						// Flags
//			NULL, 0,									// Feature level pointer and features
//			D3D11_SDK_VERSION,				// D3D11 SDK Version
//			&swapchainDescription,		// Swap chain description
//			&_swapchain,							// Out swap chain
//			&_device,									// Out device
//			NULL,						// Feature level pointer
//			&_deviceContext						// Out device context
//		)
//	);
//
//	// Create a render target view
//	ID3D11Texture2D* framebuffer{ nullptr };	// The back buffer to be attached to the render target view
//	// Get the back buffer handle from the swap chain.
//	CHECK(_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&framebuffer));
//	// Attach the back buffer handle to the render target view
//	CHECK(_device->CreateRenderTargetView(framebuffer, NULL, &_renderTargetView));
//	// Release the local-scoped back buffer, as it's attached to the render target
//	SafeRelease(&framebuffer);	
//
//	// Create a depth stencil view
//	ID3D11Texture2D* depthStencilTexture{ nullptr };
//	// 
//
//#if USE_FLIPMODE == 0	// if NOT using flip-mode
//	// Bind the render target to the device context
//	_deviceContext->OMSetRenderTargets(1, &_renderTargetView, NULL);
//#endif
//
//	return true;
//}

bool DemoApp::InitD3D() {
	CHECK(CreateDeviceResources());

	CHECK(CreateDebugLayer());

	CHECK(CreateFactory());

	CHECK(CreateSwapchain());

	CHECK(ConfigureBackBuffer());

	CHECK(ConfigureDepthStencilBuffer());

	CHECK(ConfigureRasterizer());

	CHECK(ConfigureViewport());

	InitTransformMatrices();

	InitCamera();

	return true;
}

void DemoApp::CleanD3D()
{
	_swapchain->SetFullscreenState(FALSE, NULL);	// switch to windowed mode

	// Close and release all COM objects
	SafeRelease(&_renderTargetView);
	SafeRelease(&_swapchain);
	SafeRelease(&_deviceContext);
	SafeRelease(&_device);
}

HRESULT DemoApp::ReleaseBackBuffer()
{
	return E_NOTIMPL;
}

HRESULT DemoApp::ReleaseDepthStencilBuffer()
{
	return E_NOTIMPL;
}

HRESULT DemoApp::FlushDeviceContext()
{
	return E_NOTIMPL;
}

bool DemoApp::InitPipeline()
{
	/* Vertex Shader */
	// Compile the vertex shader
	ID3DBlob* vsBlob;
	CHECK(
		CompileShaderFromFile(
			L"shaders/VertexShader.hlsl", 
			"main", 
			"vs_4_0", 
			&vsBlob
		)
	);
	
	// Create the input layout of the shader
	// Input layout descriptor
	D3D11_INPUT_ELEMENT_DESC vsInputLayoutDescriptors[] = { 
		// Position layout
		D3D11_INPUT_ELEMENT_DESC{
			.SemanticName = "POSITION",
			.SemanticIndex = 0U,
			.Format = DXGI_FORMAT_R32G32B32_FLOAT,
			.InputSlot = 0,
			.AlignedByteOffset = 0,
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0
		},
		// Color layout
		D3D11_INPUT_ELEMENT_DESC{
			.SemanticName = "COLOR",
			.SemanticIndex = 0U,
			.Format = DXGI_FORMAT_R32G32B32_FLOAT,
			.InputSlot = 0,
			.AlignedByteOffset = sizeof(Vector3),
			.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA,
			.InstanceDataStepRate = 0
		}
	};
	CHECK(
		_device->CreateInputLayout(
			vsInputLayoutDescriptors,
			(UINT)std::size(vsInputLayoutDescriptors),
			vsBlob->GetBufferPointer(), 
			vsBlob->GetBufferSize(), 
			&_inputLayout
		)
	);

	// Create the vertex shader object
	CHECK(
		_device->CreateVertexShader(
			vsBlob->GetBufferPointer(),
			vsBlob->GetBufferSize(),
			NULL,
			&_vs
		)
	);

	// Release the blob
	SafeRelease(&vsBlob);

	/* Pixel Shader */
	ID3DBlob* psBlob;
	CHECK(
		CompileShaderFromFile(
			L"shaders/PixelShader.hlsl",
			"main",
			"ps_4_0",
			&psBlob
		)
	);

	// Create the pixel shader object
	CHECK(
		_device->CreatePixelShader(
			psBlob->GetBufferPointer(),
			psBlob->GetBufferSize(),
			NULL,
			&_ps
		)
	);

	// Release the blob
	SafeRelease(&psBlob);

	return true;
}

bool DemoApp::InitVertices()
{
	// Vertex buffer
	Vertex vertices[] = {
		Vertex{
			.position = { -0.5f,  0.5f, 0.5f },
			.color = { 1.f, 0.f, 0.f }
		},
		Vertex{
			.position = {  0.5f,  0.5f, 0.5f },
			.color = { 0.f, 1.f, 0.f }
		},
		Vertex {
			.position = {  0.5f, -0.5f, 0.5f },
			.color = { 0.f, 0.f, 1.f }
		},
		Vertex{
			.position = { -0.5f, -0.5f, 0.5f },
			.color = {1.f, 0.f, 1.f}
		}
	};

	D3D11_BUFFER_DESC vertexBufferInfo{
		.ByteWidth = sizeof(Vertex) * std::size(vertices),
		.Usage = D3D11_USAGE_DYNAMIC,		// write from CPU, read from GPU
		.BindFlags = D3D11_BIND_VERTEX_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE
	};

	CHECK(_device->CreateBuffer(&vertexBufferInfo, NULL, &_vbo));

	// Copy the vertices into the buffer
	// NOTE: You can technically use ID3D11DeviceContext::UpdateSubresource 
	// to copy to a resource with any usage except D3D11_USAGE_IMMUTABLE. 
	// However, we recommend to use ID3D11DeviceContext::UpdateSubresource 
	// to update only a resource with D3D11_USAGE_DEFAULT. 
	// We recommend to use ID3D11DeviceContext::Map and 
	// ID3D11DeviceContext::Unmap to update resources with 
	// D3D11_USAGE_DYNAMIC because that is the specific purpose of 
	// D3D11_USAGE_DYNAMIC resources, and is therefore the most optimized path.
	D3D11_MAPPED_SUBRESOURCE vertexBufferSubresource;
	_deviceContext->Map(_vbo, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &vertexBufferSubresource);
	memcpy(vertexBufferSubresource.pData, vertices, sizeof(vertices));
	_deviceContext->Unmap(_vbo, NULL);

	// Create the vertex buffer
	/*D3D11_SUBRESOURCE_DATA vertexBufferData{
		.pSysMem = vertices
	};
	CHECK(
		_device->CreateBuffer(
			&vertexBufferDescriptor,
			&vertexBufferData,
			&_vbo
		)
	);*/

	_vertexBufferStride = sizeof(Vertex);
	_vertexBufferOffset = 0U;
	_vertexCount = static_cast<UINT>(std::size(vertices));

	// Index buffer
	WORD indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	D3D11_BUFFER_DESC indexBufferDescriptor{
		.ByteWidth = sizeof(WORD) * std::size(indices),
		.Usage = D3D11_USAGE_DEFAULT,
		.BindFlags = D3D11_BIND_INDEX_BUFFER,
	};

	// Create the index buffer
	D3D11_SUBRESOURCE_DATA indexBufferData{
		.pSysMem = indices
	};
	CHECK(
		_device->CreateBuffer(
			&indexBufferDescriptor,
			&indexBufferData,
			&_ibo
		)
	);

	_indexBufferStride = sizeof(WORD);
	_indexBufferOffset = 0U;
	_indexCount = static_cast<UINT>(std::size(indices));

	// Create the constant buffer
	D3D11_BUFFER_DESC constantBufferDesc{
		.ByteWidth = sizeof(ConstantBuffer),
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE
	};
	CHECK(
		_device->CreateBuffer(
			&constantBufferDesc,
			nullptr,
			&_cbo
		)
	);

	return true;
}
