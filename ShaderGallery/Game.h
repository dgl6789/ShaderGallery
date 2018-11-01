#pragma once

#include "DXCore.h"
#include "SimpleShader.h"
#include <DirectXMath.h>
#include <math.h>

#include <d3d11sdklayers.h>
#include "Lights.h"
#include "Entity.h"
#include "Camera.h"
#include <vector>
#include "BoundingBox.h"

class Mesh;
class Entity;
class Camera;
class Material;

using namespace DirectX;

class Game : public DXCore {

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);

	// Overridden mouse input helper methods
	void OnMouseDown (WPARAM buttonState, int x, int y);
	void OnMouseUp	 (WPARAM buttonState, int x, int y);
	void OnMouseMove (WPARAM buttonState, int x, int y);
	void OnMouseWheel(float wheelDelta,   int x, int y);

private:
	// Vector of active meshes
	std::vector<Mesh*> meshes;

	// Vector of active entities
	std::vector<Entity*> entities;
	std::vector<Entity*> GUIElements;

	// Vector of materials
	std::vector<Material*> materials;
	std::vector<Material*> starMaterials;

	// Vector to hold the world boundary boxes
	std::vector<BoundingBox*> worldBounds;

	// Cameras
	Camera* GameCamera;
	Camera* GUICamera;

	// Light
	DirectionalLight light;
	DirectionalLight fullBright;

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadMaterials();
	void CreateBasicGeometry();
	void DoStars();

	int starRating = -1;
	int currentStarRating = -1;
	bool canRate = false;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;
};
