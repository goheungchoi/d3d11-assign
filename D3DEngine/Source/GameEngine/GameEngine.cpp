#include "D3DEngine/GameEngine/GameEngine.h"

#include "D3DEngine/Core/Timer.h"
#include <chrono>
#include <thread>

void GameEngine::Initialize()
{
	// NOTE: Engine initialization
	// e.g., ResourceManager, FactorySingleton, etc...

}

void GameEngine::Execute() {
	Run();
}

void GameEngine::Shutdown()
{
	// NOTE: Engine Shutdown
	// e.g., free memory, singleton finalizations, etc...



	
}

void GameEngine::Run() {
	// Set timer
	Timer::InitTimer();
	float prevTime = Timer::GetTick();
	float currTime = 0.0;
	float frameTime = 0.0;
	float accumulator = 0.0;	// Helps keep track of fixed frame rate

	MSG msg = { };
	bool bQuit = false;

	// Main loop
	while (!bQuit) {
		// Handle messages on the queue
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			// The window received the destruction call
			if (msg.message == WM_QUIT) {
				bQuit = true;
			}

			// When the window is minimized, stop rendering
			if (msg.message == WM_SHOWWINDOW) {
				if (msg.lParam == SW_PARENTCLOSING)
					stop_rendering = true;
				if (msg.lParam == SW_PARENTOPENING)
					stop_rendering = false;
			}

			// Translate and dispatch message
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Update the frame delta time
		currTime = Timer::GetTick();
		frameTime = currTime - prevTime;
		prevTime = currTime;

		// Prevent updates from causing a massive game loop spiral
		if (frameTime > 0.25) frameTime = 0.25;	// Clamp the frame time

		// Update the physics
		accumulator += frameTime;

		while (accumulator >= FIXED_RATE) {
			FixedUpdate(FIXED_RATE);	// Update physics at a fixed rate
			accumulator -= FIXED_RATE;
		}

		// Update the state
		Update(frameTime);

		// Draw the current frame
		//// do not draw if we are minimized
		if (stop_rendering) {
			// Throttle the speed to avoid the endless spinning
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			continue;
		}

		// May need interpolation
		// const double alpha = accumulator / FIXED_RATE;
		// interpolate(alpha);
		Render();
	}
}
