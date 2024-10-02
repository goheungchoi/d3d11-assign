#include "camera.h"

#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")


Camera::Camera(HINSTANCE hInstance, HWND hwnd) : hInstance{hInstance}, hwnd{hwnd}
{
	HRESULT res = S_OK;
	res = DirectInput8Create(hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&DirectInput,
		NULL
	);

	res = DirectInput->CreateDevice(GUID_SysKeyboard, &DIKeyboard, NULL);

	res = DirectInput->CreateDevice(GUID_SysMouse, &DIMouse, NULL);

	res = DIKeyboard->SetDataFormat(&c_dfDIKeyboard);
	res = DIKeyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	res = DIMouse->SetDataFormat(&c_dfDIMouse);
	res = DIMouse->SetCooperativeLevel(hwnd, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);
}

void Camera::MoveLeftRight(float move)
{
	leftRightMove = move;
}

void Camera::MoveBackForward(float move)
{
	backForwardMove = move;
}

void Camera::RotateAroundXAxis(float degrees)
{
	pitch += XMConvertToRadians(degrees);
}

void Camera::RotateAroundYAxis(float degrees)
{
	yaw += XMConvertToRadians(degrees);
}

void Camera::Update(float dt)
{
	DIMOUSESTATE mouseCurrState;

	BYTE keyboardState[256];

	DIKeyboard->Acquire();
	DIMouse->Acquire();

	DIMouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);

	DIKeyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);

	if (keyboardState[DIK_ESCAPE] & 0x80)
		PostMessage(hwnd, WM_DESTROY, 0, 0);

	float speed = 100.0f * dt;

	if (keyboardState[DIK_Q] & 0x80)
	{
		downUpMove -= speed;
	}
	if (keyboardState[DIK_E] & 0x80)
	{
		downUpMove += speed;
	}
	if (keyboardState[DIK_A] & 0x80)
	{
		leftRightMove -= speed;
	}
	if (keyboardState[DIK_D] & 0x80)
	{
		leftRightMove += speed;
	}
	if (keyboardState[DIK_W] & 0x80)
	{
		backForwardMove += speed;
	}
	if (keyboardState[DIK_S] & 0x80)
	{
		backForwardMove -= speed;
	}
	if ((mouseCurrState.lX != mouseLastState.lX) || (mouseCurrState.lY != mouseLastState.lY))
	{
		yaw += mouseLastState.lX * 0.001f;
		pitch += mouseCurrState.lY * 0.001f;
		mouseLastState = mouseCurrState;
	}

	return;
}

XMMATRIX Camera::GetViewTransform()
{
	// Move
	position += downUpMove * up; downUpMove = 0.f;
	position += leftRightMove * right; leftRightMove = 0.f;
	position += backForwardMove * forward; backForwardMove = 0.f;

	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
	right = XMVector3TransformCoord(RIGHT, rotation);
	forward = XMVector3TransformCoord(FORWARD, rotation);

	return XMMatrixLookToLH(position, forward, UP);
}
