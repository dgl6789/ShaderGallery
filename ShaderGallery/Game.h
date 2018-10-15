#pragma once

#include "DXCore.h"
#include "SimpleShader.h"
#include <DirectXMath.h>

#include "Entity.h"
#include "Camera.h"
#include <vector>

class Mesh;
class Entity;
class Camera;
class Material;

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

	// Vector of materials
	std::vector<Material*> materials;

	// Camera
	Camera* GameCamera;

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadMaterials();
	void CreateBasicGeometry();

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;
};