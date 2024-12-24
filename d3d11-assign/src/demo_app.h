#pragma once

#include "common.h"

#include "D3DEngine/GameEngine/GameEngine.h"

#include "types.h"

class DemoApp : public GameEngine
{
	using Super = GameEngine;

#ifndef NDEBUG 
	float frameTime{ 0.f };
	std::size_t fps{ 0 };
	std::size_t count{ 0 };
#endif

	class D3D11Renderer* _renderer;
	
	class Camera* _camera;
  
	class Model* _model;
  class Animation* _animation;
  class Animator* _animator;

	cbTransformConstants _transformConstants;
	cbShadingConstants _shadingConstants;

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

private:

	void InitImgui();

};
