// Timer.h
#pragma once

#include "D3DEngine/EngineCommon.h"

class Timer {
	static float _precision;
public:
	static void InitTimer();
	static float GetTick();
};
