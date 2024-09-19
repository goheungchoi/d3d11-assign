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

	class D3D11Renderer* _renderer;
	class Model* model;
	class Camera* _camera;
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

	XMMATRIX _proj;
	XMMATRIX _view;
	cbPerFrame _cbData{};

	void InitTransformMatrices();
	void InitCamera();
	void InitModels();
	void InitLights();


private:

	void InitImgui();

};
