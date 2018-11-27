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

	ID3D11RasterizerState * rast;
	ID3D11BlendState* blend;

	//post process stuff
	ID3D11SamplerState* sampleState;
	D3D11_SAMPLER_DESC* sampleDescription;
	ID3D11ShaderResourceView* finalSRV;		// Allows us to sample from the same texture
	ID3D11RenderTargetView* finalRTV;		// Allows us to sample from the same texture
	ID3D11RenderTargetView* blurRTV;		// Allows us to render to a texture
	ID3D11ShaderResourceView* blurSRV;		// Allows us to sample from the same texture
	ID3D11RenderTargetView* blur2RTV;		// Allows us to render to a texture
	ID3D11ShaderResourceView* blur2SRV;		// Allows us to sample from the same texture

	SimplePixelShader* addBlendPS;
	SimplePixelShader* blurPS;
	SimpleVertexShader* ppVS;

	// Vector of active meshes
	std::vector<Mesh*> meshes;

	// Vector of active entities
	std::vector<Entity*> entities;
	std::vector<Entity*> exhibits;
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
	void DoExhibits();

	int starRating = -1;
	int currentStarRating = -1;
	int currentExhibit;
	bool canRate = false;
	bool isRating = false;

	// Keeps track of the old mouse position.  Useful for 
	// determining how far the mouse moved in a single frame.
	POINT prevMousePos;
};
