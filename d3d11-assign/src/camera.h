#pragma once

#include "types.h"

#include <directxtk/SimpleMath.h>
using namespace DirectX;
using namespace DirectX::SimpleMath;

#include <dinput.h>

class Camera {
	static constexpr XMVECTOR UP{ 0.f, 1.f, 0.f, 0.f };
	static constexpr XMVECTOR RIGHT{ -1.f, 0.f, 0.f, 0.f };
	static constexpr XMVECTOR FORWARD{ 0.f, 0.f, -1.f, 0.f };

	HINSTANCE hInstance; HWND hwnd;

	DIMOUSESTATE mouseLastState{};
	LPDIRECTINPUT8 DirectInput{};

	float rotx = 0;
	float rotz = 0;
	float scaleX = 1.0f;
	float scaleY = 1.0f;

	float moveSpeed{100.f};
  float rotationSpeed{1.f};

	float downUpMove{ 0.f };
	float leftRightMove{ 0.f };
	float backForwardMove{ 0.f };

	XMVECTOR position{ (__m128)g_camPos };
	XMVECTOR forward{ 0.f, 0.f, -1.f, 0.f };
	XMVECTOR up{ 0.f, 1.f, 0.f, 0.f };
	XMVECTOR right{ -1.f, 0.f, 0.f, 0.f };
	float pitch{ 0.f };	// around x-axis in radian
	float yaw{ 0.f };	// around y-axis in radian
	float roll{ 0.f };	// around z-axis in radian. Must be 0.f
public:
	
	Camera(HINSTANCE hInstance, HWND hwnd);
	
	void AddMoveSpeed(float speed);
  void AddRotationSpeed(float speed);

	void MoveDownUp(float move);
	void MoveLeftRight(float move);
	void MoveBackForward(float move);

	void RotateAroundXAxis(float degrees);
	void RotateAroundYAxis(float degrees);

	XMMATRIX GetViewTransform();
};

