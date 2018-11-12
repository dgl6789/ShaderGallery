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

	GUICamera = new Camera(0, 0, -5);
	GUICamera->MakeGUI();
	GUICamera->UpdateProjectionMatrix((float)width / 100, (float)height / 100);

	meshes = { };
	entities = { };
	materials = { };
	starMaterials = { };

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
	for (auto& m : starMaterials) delete m;
	for (auto& e : entities) delete e;
	for (auto& w : worldBounds) delete w;
	for (auto& g : GUIElements) delete g;

	delete GameCamera;
	delete GUICamera;
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
	

	// Game Objects
	meshes.push_back(new Mesh(device, "../../Assets/Models/helix.obj"));
	meshes.push_back(new Mesh(device, "../../Assets/Models/cube_inverted.obj"));

	//GUI Mesh
	//meshes.push_back(new Mesh(device, "../../Assets/Models/cube.obj"));
	meshes.push_back(new Mesh(device, "../../Assets/Models/plane.obj"));


	entities.push_back(new Entity(meshes[0], materials[0], context));
	entities.push_back(new Entity(meshes[1], materials[1], context));
	//entities.push_back(new Entity(meshes[1], materials[1], context));

	GUIElements.push_back(new Entity(meshes[2], starMaterials[0], context));
	GUIElements[0]->SetRotation(XMFLOAT3(-(3.141592654f / 2), 0, 0));
	GUIElements[0]->SetScale(XMFLOAT3(2.56f, .5f, 0.5f));


	// Define the world boundaries
	worldBounds.push_back(new BoundingBox(XMFLOAT2(0.0f, 0.0f), XMFLOAT2(5.0f, 5.0f)));
	worldBounds.push_back(new BoundingBox(XMFLOAT2(0.0f, 7.5f), XMFLOAT2(2.5f, 5.0f)));

	//make walls "visible" by adding an inverted box to represent their dimensions
	entities[1]->SetScale(XMFLOAT3(worldBounds[0]->GetHalfSize().x + 0.2f, 3.0f, worldBounds[0]->GetHalfSize().y + 0.2f));
	entities[1]->SetPosition(XMFLOAT3(worldBounds[0]->GetCenter().x, 0.0f, worldBounds[0]->GetCenter().y));

	//entities[2]->SetScale(XMFLOAT3(worldBounds[1]->GetHalfSize().x + 0.2f, 3.0f, worldBounds[0]->GetHalfSize().y + 0.2f));
	//entities[2]->SetPosition(XMFLOAT3(worldBounds[1]->GetCenter().x, 0.0f, worldBounds[0]->GetCenter().y));

	light.AmbientColor = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	light.DiffuseColor = XMFLOAT4(1, 1, 1, 1);
	light.Direction = XMFLOAT3(1, -1, 0);

	fullBright.AmbientColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	fullBright.DiffuseColor = XMFLOAT4(1, 1, 1, 1);
	fullBright.Direction = XMFLOAT3(1, -1, 0);

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
	//IT IS NECESSARY FOR ALL MATERIALS TO CALL THE SetSpecularMap(); METHOD
	//IF NO SPECULAR MAP EXISTS FOR A MATERIAL, SET IT USING THE NO_SPEC.png FILE WITHIN THE TEXTURES FOLDER

	// Lava Texture
	materials.push_back(new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context)));
	materials[0]->SetTexture(device, context, L"../../Assets/Textures/Lava_005_COLOR.jpg");
	materials[0]->SetSpecularMap(device, context, L"../../Assets/Textures/spec.jpg");
	materials[0]->SetNormalMap(device, context, L"../../Assets/Textures/panel_normal.png");
	materials[0]->GetVertexShader()->LoadShaderFile(L"VertexShader.cso");
	materials[0]->GetPixelShader()->LoadShaderFile(L"PixelShader.cso");

	// Panel Texture
	materials.push_back(new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context)));
	materials[1]->SetTexture(device, context, L"../../Assets/Textures/panel_normal.png");
	materials[1]->SetSpecularMap(device, context, L"../../Assets/Textures/spec.jpg");
	materials[1]->SetNormalMap(device, context, L"../../Assets/Textures/panel_normal.png");
	materials[1]->GetVertexShader()->LoadShaderFile(L"VertexShader.cso");
	materials[1]->GetPixelShader()->LoadShaderFile(L"PixelShader.cso");
	
	//loop through all the ui star materials
	for (int i = 0; i < 6; i++) {
		starMaterials.push_back(new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context)));

		//concatenate a wstring then reference its first index to get a wchar_t* object
		std::wstring w_file = L"../../Assets/Textures/UI/ui_starTray_";
		w_file += std::to_wstring(i);
		w_file += L".png";

		starMaterials[i]->SetTexture(device, context, &w_file[0]);
		starMaterials[i]->SetSpecularMap(device, context, L"../../Assets/Textures/NO_SPEC.png");
		starMaterials[i]->SetNormalMap(device, context, L"../../Assets/Textures/panel_normal.png");
		starMaterials[i]->GetVertexShader()->LoadShaderFile(L"VertexShader.cso");
		starMaterials[i]->GetPixelShader()->LoadShaderFile(L"PixelShader.cso");
	}
	
}

// --------------------------------------------------------
// Creates three primitive shapes for testing and adds them to the mesh list
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{

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
	if(GUICamera)
		GUICamera->UpdateProjectionMatrix((float)width / 100, (float)height / 100);
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Quit if the escape key is pressed
	if (GetAsyncKeyState(VK_ESCAPE))
		Quit();

	//if(prevMousePos.y > (float))

	//rotate the helix that exists in entities
	//entities[0]->SetRotation(XMFLOAT3(0, totalTime * 0.5f, 0));
	GUIElements[0]->SetPosition(XMFLOAT3((float)width / (2 * 100), 1, 2));

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
	XMFLOAT3 newestPosition;
	XMStoreFloat3(&newestPosition, DirectX::XMVectorLerp(XMLoadFloat3(&prevPosition), XMLoadFloat3(&GameCamera->GetPosition()), .5f));

	GameCamera->SetPosition(newestPosition);


	float distance = sqrt(
		pow((GameCamera->GetPosition().x - entities[0]->GetPosition().x), 2)
		+ pow((GameCamera->GetPosition().z - entities[0]->GetPosition().z), 2));
	canRate = false;
	if (distance < 4) {
		canRate = true;
		DoStars();
	}
}

//calculate stars for game rating system
void Game::DoStars()
{
	starRating = -1;
	if (currentStarRating != -1) return;
	//if mouse too high then get out of there.
	if (prevMousePos.y < (float)height * (2.0f / 3)) {
		GUIElements[0]->SetMaterial(starMaterials[0]);
		return;
	}
	
	float left_start = (float)width / 2 - 256;
	float right_end = (float)width / 2 + 256;
	int increment = 512 / 5;

	if (prevMousePos.x < left_start) {
		GUIElements[0]->SetMaterial(starMaterials[0]);
	}else if (prevMousePos.x >= right_end - 3) {
		GUIElements[0]->SetMaterial(starMaterials[5]);
	}else {
		starRating = (int)((prevMousePos.x - left_start) / increment) + 1;
		GUIElements[0]->SetMaterial(starMaterials[starRating]);
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
		entities[i]->GetMaterial()->GetPixelShader()->SetFloat3("cameraPosition", GameCamera->GetPosition());
		entities[i]->Render(GameCamera->GetView(), GameCamera->GetProjection());
	}
	

	if (canRate) {
		for (int i = 0; i < GUIElements.size(); i++) {
			GUIElements[i]->GetMaterial()->GetPixelShader()->SetData("light", &fullBright, sizeof(DirectionalLight));
			entities[i]->GetMaterial()->GetPixelShader()->SetFloat3("cameraPosition", GUICamera->GetPosition());
			GUIElements[i]->Render(GUICamera->GetView(), GUICamera->GetProjection());
		}
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
	if (canRate && starRating != -1) {
		currentStarRating = starRating;
		GUIElements[0]->SetMaterial(starMaterials[currentStarRating]);
	}
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