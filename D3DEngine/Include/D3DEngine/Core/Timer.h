// Timer.h
#pragma once

#include "D3DEngine/EngineCommon.h"

class Timer {
	static double _precision;
public:
	static void InitTimer();
	static double GetTick();
};
