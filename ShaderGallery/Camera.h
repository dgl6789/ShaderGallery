#pragma once

#include "Game.h"
using namespace DirectX;

class Camera
{
public:
	Camera(float x, float y, float z);
	~Camera();

	// Transformations
	void TranslateBy(float x, float y, float z);
	void TranslateTo(float x, float y, float z);
	void RotateBy(float x, float y);

	// Updating
	void Update(float dt);
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float aspectRatio);
	void UpdateProjectionMatrix(float width, float height);

	// Make Camera Orthographic GUI
	void MakeGUI();


	// Getters
	XMFLOAT3 GetPosition() { return position; }
	XMFLOAT4X4 GetView() { return viewMatrix; }
	XMFLOAT4X4 GetProjection() { return projMatrix; }

	void SetPosition(XMFLOAT3 newPosition);

	// Properties
	float mouseSensitivity = 0.005f;
	float GetMouseSensitivity() { return mouseSensitivity; }

private:
	// Is GUI?
	bool GUI = false;

	// Camera matrices
	XMFLOAT4X4 viewMatrix;
	XMFLOAT4X4 projMatrix;

	// Transformations
	XMFLOAT3 position;
	XMFLOAT4 rotation;
	float xRotation;
	float yRotation;
};