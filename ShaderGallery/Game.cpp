#include "Game.h"
#include "Vertex.h"

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"ShaderGallery",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true)			   // Show extra stats (fps) in title bar?
{
	// Initialize fields
	GameCamera = new Camera(0, 0, -5);
	GameCamera->UpdateProjectionMatrix((float)width / height);

	meshes = { };
	entities = { };
	materials = { };

#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	// CreateConsoleWindow(500, 120, 32, 120);
	// printf("Console window created successfully.  Feel free to printf() here.");
#endif
	
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	// Delete each added resource
	for (auto& m : meshes) delete m;
	for (auto& m : materials) delete m;
	for (auto& e : entities) delete e;
	for (auto& w : worldBounds) delete w;

	delete GameCamera;
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadMaterials();
	
	
	//CreateBasicGeometry();
	
	// Create the entities that we'll draw
	/*
	entities.push_back(new Entity(meshes[2], materials[0], context));
	entities.push_back(new Entity(meshes[0], materials[0], context));
	entities.push_back(new Entity(meshes[0], materials[0], context));
	entities.push_back(new Entity(meshes[0], materials[0], context));
	entities.push_back(new Entity(meshes[0], materials[0], context));
	entities.push_back(new Entity(meshes[0], materials[0], context));
	entities.push_back(new Entity(meshes[0], materials[0], context));
	entities.push_back(new Entity(meshes[0], materials[0], context));
	entities.push_back(new Entity(meshes[0], materials[0], context));
	entities.push_back(new Entity(meshes[0], materials[0], context));
	entities.push_back(new Entity(meshes[0], materials[0], context));
	entities.push_back(new Entity(meshes[0], materials[0], context));
	entities.push_back(new Entity(meshes[0], materials[0], context));
	*/

	meshes.push_back(new Mesh(device, "../../Assets/Models/helix.obj"));

	entities.push_back(new Entity(meshes[0], materials[0], context));

	// Define the world boundaries
	worldBounds.push_back(new BoundingBox(XMFLOAT2(0.0f, 0.0f), XMFLOAT2(5.0f, 5.0f)));
	worldBounds.push_back(new BoundingBox(XMFLOAT2(0.0f, 7.5f), XMFLOAT2(2.5f, 5.0f)));

	light.AmbientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	light.DiffuseColor = XMFLOAT4(1, 1, 1, 1);
	light.Direction = XMFLOAT3(1, -1, 0);

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files using
// my SimpleShader wrapper for DirectX shader manipulation.
// - SimpleShader provides helpful methods for sending
//   data to individual variables on the GPU
// --------------------------------------------------------
void Game::LoadMaterials() {
	materials.push_back(new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context)));
	materials[0]->SetTexture(device, context, L"../../Assets/Textures/Lava_005_COLOR.jpg");
	materials[0]->GetVertexShader()->LoadShaderFile(L"VertexShader.cso");
	materials[0]->GetPixelShader()->LoadShaderFile(L"PixelShader.cso");
}

// --------------------------------------------------------
// Creates three primitive shapes for testing and adds them to the mesh list
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in memory
	//    over to a DirectX-controlled data structure (the vertex buffer)
	/*
	Vertex verticesTriangle[] =
	{
		{ XMFLOAT3(0.0f,  0.35f,  0.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(0.5f,  -0.5f,  0.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(-0.5f,  -0.5f,  0.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
	};

	Vertex verticesSquare[] = 
	{ 
		{ XMFLOAT3(-1.8f,   -0.5f,  0.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(-1.8f,   0.35f,  0.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(-0.95f,   0.35f,  0.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-0.95f,  -0.5f,  0.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
	};
	
	Vertex verticesCircle[] = 
	{ 
		{ XMFLOAT3(    0.0f,     0.0f,  0.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3( 0.4045f,  0.2935f,  0.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3( 0.1545f,  0.4755f,  0.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-0.1545f,  0.4755f,  0.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT3(-0.4045f,  0.2935f,  0.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(   -0.5f,	 0.0f,  0.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(-0.4045f, -0.2935f,  0.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(-0.1545f, -0.4755f,  0.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3( 0.1545f, -0.4755f,  0.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3( 0.4045f, -0.2935f,  0.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(    0.5f,     0.0f,  0.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
	};

	// Set up the indices, which tell us which vertices to use and in which order
	// - This is somewhat redundant for just 3 vertices (it's a simple example)
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	int indicesTriangle[] = { 0, 1, 2 };
	int indicesSquare[] = { 0, 1, 2, 3, 0, 2 };
	int indicesCircle[] = { 0, 2, 1, 0, 3, 2, 0, 4, 3, 0, 5, 4, 0, 6, 5, 0, 7, 6, 0, 8, 7, 0, 9, 8, 0, 10, 9, 0, 1, 10 };

	// Initialize the mesh and send it to the mesh list

	meshes.push_back(new Mesh(device, verticesTriangle, indicesTriangle, 3));
	meshes.push_back(new Mesh(device, verticesSquare, indicesSquare, 6));
	meshes.push_back(new Mesh(device, verticesCircle, indicesCircle, 30));
	*/
}

// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	// Update the projection matrix assuming the
	// camera exists
	if (GameCamera)
		GameCamera->UpdateProjectionMatrix((float)width / height);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	/*
	// Assignment 3
	// Make a circle in the center that rotates
	float sinTime = 2.0f +  sin(totalTime * 5.0f) / 5.0f;
	float radius = sinTime + 1.0f;

	entities[0]->SetRotation(XMFLOAT3(0, 0, -totalTime * 0.5f));
	entities[0]->SetScale(XMFLOAT3(2, 2, 2));

	// Make a bunch of triangles that revolve around the circle
	for (int i = 1; i < entities.size(); i++) {
		entities[i]->SetScale(XMFLOAT3(0.05f * i, 0.05f * i, 0.05f * i));
		entities[i]->SetRotation(XMFLOAT3(0, 0, -totalTime * 0.5f + (i * 10)));
		entities[i]->SetPosition(XMFLOAT3(sin(totalTime * 0.75f + (i * 0.15f)) * 2.0f, 0, cos(totalTime * 0.75f + (i * 0.15f)) * 2.0f));
	}*/

	//rotate the helix that exists in entities
	entities[0]->SetRotation(XMFLOAT3(0, totalTime * 0.5f, 0));

	// Movement
	XMFLOAT3 prevPosition = GameCamera->GetPosition();	// Position before the move
	GameCamera->Update(deltaTime);

	XMFLOAT3 newPosition = GameCamera->GetPosition();	// Position after the move

	bool willBeInBox = false;
	int cameraCurBox = -1;
	for (int i = 0; i < worldBounds.size(); i++)
	{
		// Was the player in this box before moving?
		if (worldBounds[i]->PointInside(prevPosition))
		{
			cameraCurBox = i;
		}

		// Is the player in this box after moving?
		if (worldBounds[i]->PointInside(newPosition))
		{
			// If so, we don't need to bother checking if they're outside of the rest of the boxes; they're in bounds
			willBeInBox = true;
			break;
		}
	}

	// If the player's move will put them out of bounds, make sure they stay in their current box!
	if (!willBeInBox)
	{
		// Failsafe, in case the player somehow started the frame out of bounds
		if (cameraCurBox == -1)
		{
			cameraCurBox = 0;
		}

		// Adjust the player's new position to the edge of their box
		newPosition.x = worldBounds[cameraCurBox]->VectorToEdge(newPosition).x;
		newPosition.z = worldBounds[cameraCurBox]->VectorToEdge(newPosition).z;
		GameCamera->TranslateTo(newPosition.x, 0.0f, newPosition.z);
	}
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Background color (Black in this case) for clearing
	const float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(backBufferRTV, color);
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);

	
	// Set buffers in the input assembler
	//  - Do this ONCE PER OBJECT you're drawing, since each object might
	//    have different geometry.
	for (int i = 0; i < entities.size(); i++) {
		entities[i]->GetMaterial()->GetPixelShader()->SetData("light", &light, sizeof(DirectionalLight));
		entities[i]->Render(GameCamera->GetView(), GameCamera->GetProjection());
	}

	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);
}

#pragma region Mouse Input

// --------------------------------------------------------
// Helper method for mouse clicking.  We get this information
// from the OS-level messages anyway, so these helpers have
// been created to provide basic mouse input if you want it.
// --------------------------------------------------------
void Game::OnMouseDown(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;

	// Caputure the mouse so we keep getting mouse move
	// events even if the mouse leaves the window.  we'll be
	// releasing the capture once a mouse button is released
	SetCapture(hWnd);
}

// --------------------------------------------------------
// Helper method for mouse release
// --------------------------------------------------------
void Game::OnMouseUp(WPARAM buttonState, int x, int y)
{
	// Add any custom code here...

	// We don't care about the tracking the cursor outside
	// the window anymore (we're not dragging if the mouse is up)
	ReleaseCapture();
}

// --------------------------------------------------------
// Helper method for mouse movement.  We only get this message
// if the mouse is currently over the window, or if we're 
// currently capturing the mouse.
// --------------------------------------------------------
void Game::OnMouseMove(WPARAM buttonState, int x, int y)
{
	// Check left mouse button
	if (buttonState & 0x0001)
	{
		float xDiff = (x - prevMousePos.x) * GameCamera->GetMouseSensitivity();
		float yDiff = (y - prevMousePos.y) * GameCamera->GetMouseSensitivity();
		GameCamera->RotateBy(yDiff, xDiff);
	}

	// Save the previous mouse position, so we have it for the future
	prevMousePos.x = x;
	prevMousePos.y = y;
}

// --------------------------------------------------------
// Helper method for mouse wheel scrolling.  
// WheelDelta may be positive or negative, depending 
// on the direction of the scroll
// --------------------------------------------------------
void Game::OnMouseWheel(float wheelDelta, int x, int y)
{
	// Add any custom code here...
}
#pragma endregion