// Timer.cc
#include "D3DEngine/Core/Timer.h"

float Timer::_precision = 0.0;

void Timer::InitTimer() {
	LARGE_INTEGER freq;
	if (!QueryPerformanceFrequency(&freq)) {
		// TODO: Need a logger!
		throw std::runtime_error("Timer: Performance counter is not supported!");
	}
	_precision = 1.0 / static_cast<double>(freq.QuadPart);
}

float Timer::GetTick() {
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return static_cast<float>(counter.QuadPart) * _precision;
}
