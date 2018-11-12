#include "Camera.h"

Camera::Camera(float x, float y, float z)
{
	position = XMFLOAT3(x, y, z);

	XMStoreFloat4(&rotation, XMQuaternionIdentity());
	xRotation = 0;
	yRotation = 0;

	XMStoreFloat4x4(&viewMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&projMatrix, XMMatrixIdentity());
}

Camera::~Camera() { }

// Shifts the camera position an amount equal to the input values
void Camera::TranslateBy(float x, float y, float z) {
	
	// Rotate the desired movement vector based on the rotation of the camera
	XMVECTOR dir = XMVector3Rotate(XMVectorSet(x, y, z, 0), XMLoadFloat4(&rotation));


	// Move in the calculated direction
	XMStoreFloat3(&position, XMLoadFloat3(&position) + dir);
	position.y = 0;
}

// Moves the camera to the location given by the input values
void Camera::TranslateTo(float x, float y, float z)
{
	// Simple addition
	position.x += x;
	position.y += y;
	position.z += z;
}

// Rotate on the X and/or Y axis
void Camera::RotateBy(float x, float y)
{
	// Adjust the rotation
	xRotation += x;
	yRotation += y;

	// Clamp the x between PI / 4 and -PI / 4
	xRotation = max(min(xRotation, XM_PIDIV4), -XM_PIDIV4);

	// Store the rotation matrix
	XMStoreFloat4(&rotation, XMQuaternionRotationRollPitchYaw(xRotation, yRotation, 0));
}

// Camera's update, which looks for key presses
void Camera::Update(float dt)
{
	if (GUI) return;
	// Current speed
	float speed = dt * 3;

	// Speed up or down as necessary
	if (GetAsyncKeyState(VK_SHIFT)) { speed *= 5; }
	if (GetAsyncKeyState(VK_CONTROL)) { speed *= 0.1f; }

	// Movement
	if (GetAsyncKeyState('W') & 0x8000) { TranslateBy(0, 0, speed); }
	if (GetAsyncKeyState('S') & 0x8000) { TranslateBy(0, 0, -speed); }
	if (GetAsyncKeyState('A') & 0x8000) { TranslateBy(-speed, 0, 0); }
	if (GetAsyncKeyState('D') & 0x8000) { TranslateBy(speed, 0, 0); }
	//if (GetAsyncKeyState('X') & 0x8000) { TranslateBy(0, -speed, 0); }
	//if (GetAsyncKeyState(' ') & 0x8000) { TranslateBy(0, speed, 0); }

	// Update the view every frame - could be optimized
	UpdateViewMatrix();
}

// Creates a new view matrix based on current position and orientation
void Camera::UpdateViewMatrix()
{
	if (GUI) return;
	// Rotate the standard "forward" matrix by our rotation
	// This gives us our "look direction"
	XMVECTOR dir = XMVector3Rotate(XMVectorSet(0, 0, 1, 0), XMLoadFloat4(&rotation));

	XMMATRIX view = XMMatrixLookToLH(
		XMLoadFloat3(&position),
		dir,
		XMVectorSet(0, 1, 0, 0));

	XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(view));
}

// Updates the projection matrix
void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	if (GUI) return;
	XMMATRIX P = XMMatrixPerspectiveFovLH(
		0.25f * XM_PI,		// Field of View Angle
		aspectRatio,		// Aspect ratio
		0.1f,				// Near clip plane distance
		100.0f);			// Far clip plane distance
	XMStoreFloat4x4(&projMatrix, XMMatrixTranspose(P)); // Transpose for HLSL!
}

void Camera::UpdateProjectionMatrix(float width, float height)
{
	if (!GUI) return;
	XMStoreFloat4x4(&projMatrix, XMMatrixTranspose(XMMatrixOrthographicOffCenterLH(0.0f, width, 0.0f, height, 0.0f, 100.0f)));
}

void Camera::MakeGUI()
{
	// Set up the view
	//XMStoreFloat4x4(&viewMatrix, XMMatrixTranspose(XMMatrixIdentity()));
	XMStoreFloat4x4(&projMatrix, XMMatrixTranspose(XMMatrixOrthographicOffCenterLH(0.0f, 1920.0f, 0.0f, 1080.0f, 0.0f, 100.0f)));
	GUI = true;
}

void Camera::SetPosition(XMFLOAT3 newPosition)
{
	position = newPosition;
}
