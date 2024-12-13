#include "camera.h"

#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")


Camera::Camera(HINSTANCE hInstance, HWND hwnd) : hInstance{hInstance}, hwnd{hwnd}
{

}

void Camera::AddMoveSpeed(float speed) { this->moveSpeed += speed; }

void Camera::AddRotationSpeed(float speed) { this->rotationSpeed += speed; }

void Camera::MoveDownUp(float move) { downUpMove += move; }

void Camera::MoveLeftRight(float move)
{
	leftRightMove += move;
}

void Camera::MoveBackForward(float move)
{
	backForwardMove += move;
}

void Camera::RotateAroundXAxis(float degrees)
{
	pitch += XMConvertToRadians(degrees);
}

void Camera::RotateAroundYAxis(float degrees)
{
	yaw += XMConvertToRadians(degrees);
}

XMMATRIX Camera::GetViewTransform()
{
	// Move
  position += moveSpeed * downUpMove * up; downUpMove = 0.f;
	position += moveSpeed * leftRightMove * right; leftRightMove = 0.f;
	position += moveSpeed * backForwardMove * forward; backForwardMove = 0.f;

	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(-pitch, yaw, roll);
	right = XMVector3TransformCoord(RIGHT, rotation);
	forward = XMVector3TransformCoord(FORWARD, rotation);

	return XMMatrixLookToLH(position, forward, UP);
}
