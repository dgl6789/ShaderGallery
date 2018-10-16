#pragma once

#include "DXCore.h"
#include "SimpleShader.h"
#include <DirectXMath.h>

#include <d3d11sdklayers.h>
#include "WICTextureLoader.h"
#include "Lights.h"
#include "Entity.h"
#include "Camera.h"
#include <vector>

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

	// Vector of materials
	std::vector<Material*> materials;

	// Camera
	Camera* GameCamera;

	// Light
	DirectionalLight light;

	// Texture Sampling
	ID3D11ShaderResourceView * pSRV;
	ID3D11SamplerState * pSampleState;
	D3D11_SAMPLER_DESC * pSampleDescription;

	// Initialization helper methods - feel free to customize, combine, etc.
	void LoadMaterials();
	void LoadTextures();
	void CreateTextureResources();
	void CreateBasicGeometry();

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;
};
