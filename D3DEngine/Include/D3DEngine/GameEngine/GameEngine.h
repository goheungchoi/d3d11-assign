#pragma once

#include "D3DEngine/EngineCommon.h"

class GameEngine {
public:
	AppWindow hWindow;
	bool stop_rendering{ false };

	class D2DRenderer* d2d{ nullptr };
	class InputSystem* inputSystem{ nullptr };

	virtual void Initialize();
	virtual void Execute();
	virtual void Shutdown();

protected:
	virtual void FixedUpdate(float) = 0;
	virtual void Update(float) = 0;
	virtual void Render() = 0;

private:
	void Run();
};
