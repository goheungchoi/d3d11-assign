#pragma once

#include "Renderer/Internal/D3D11Common.h"
#include "Renderer/Internal/D3D11Types.h"

namespace DX {

class RenderContext {

	class RenderDevice& _device;

  ComPtr<ID3D11DeviceContext> _deferredContext;
  ComPtr<ID3D11CommandList> _commandList;

	RenderContext(class RenderDevice& device) : _device{device} {}

	friend class RenderDevice;

public:

	ID3D11DeviceContext* Get() { return _deferredContext.Get(); }

	void StartCommandList() { 
		_commandList.Reset();
	}

	// TODO: Subresource update

	void FinishCommandList() {
		_deferredContext->FinishCommandList(FALSE, _commandList.GetAddressOf());
	}
	
};


}

