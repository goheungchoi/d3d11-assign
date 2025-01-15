#pragma once

#include "D3DEngine/EngineCommon.h"

#include "D3DEngine/Core/MathUtils.h"

#include "D3DEngine/Core/Handle.h"

namespace DX {
class LightData;
}

class IRenderer {

 public:
  virtual void Initialize(HWND hWnd, UINT width, UINT height,
                  bool allowTearing = false) = 0;
  virtual void Shutdown() = 0;

  virtual void BeginFrame(XMMATRIX view, XMMATRIX proj) = 0;
  virtual void BeginDraw() = 0;

	// Resource bindings
  virtual void ScheduleMesh(Handle renderMeshHandle, XMMATRIX transform) = 0;
  virtual void ScheduleLight(Handle lightHandle, XMVECTOR components) = 0;
  
	virtual void DrawOpaqueMeshes() = 0;
  virtual void DrawShadows() = 0;
  virtual void DrawLights() = 0;
	virtual void DrawImGui() = 0;

  virtual void EndDraw() = 0;
  virtual void EndFrame() = 0;



	


	// Resource management
  virtual Handle CreateShader(Handle shaderHandle) = 0;
  virtual Handle CreateTexture(Handle textureHandle) = 0;
  virtual Handle CreateMesh(Handle meshHandle) = 0;
  virtual Handle CreateLight(const DX::LightData* light) = 0;
};
