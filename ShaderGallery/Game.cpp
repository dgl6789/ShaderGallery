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
	for (auto& e : exhibits) delete e;
	for (auto& w : worldBounds) delete w;
	for (auto& e : exhibitBounds) delete e;
	for (auto& g : GUIElements) delete g;

	blend->Release();
	rast->Release();
	delete GameCamera;
	delete GUICamera;

	delete addBlendPS;
	delete blurPS;
	delete ppVS;

	finalSRV->Release();
	finalRTV->Release();

	blurSRV->Release();
	blurRTV->Release();

	blur2SRV->Release();
	blur2RTV->Release();
	
	skySRV->Release();
	skyRasterizerState->Release();
	skyDepthState->Release();
	delete skyMesh;

	delete skyPixelShader;
	delete skyVertexShader;
	
	delete sampleDescription;
	sampleState->Release();
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	sampleDescription = new D3D11_SAMPLER_DESC();
	sampleDescription->AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampleDescription->AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampleDescription->AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	sampleDescription->BorderColor[0] = 0.0f;
	sampleDescription->BorderColor[1] = 0.0f;
	sampleDescription->BorderColor[2] = 0.0f;
	sampleDescription->BorderColor[3] = 0.0f;
	sampleDescription->ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampleDescription->Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampleDescription->MaxAnisotropy = 0;
	sampleDescription->MaxLOD = D3D11_FLOAT32_MAX;
	sampleDescription->MinLOD = 0;
	sampleDescription->MipLODBias = 0;
	device->CreateSamplerState(sampleDescription, &sampleState);

	//Post Process stuff
	addBlendPS = new SimplePixelShader(device, context);
	addBlendPS->LoadShaderFile(L"AddBlendPS.cso");

	blurPS = new SimplePixelShader(device, context);
	blurPS->LoadShaderFile(L"BlurPS.cso");

	ppVS = new SimpleVertexShader(device, context);
	ppVS->LoadShaderFile(L"PostProcessVS.cso");

	// Create post process resources -----------------------------------------
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;

	ID3D11Texture2D* ppTexture;
	ID3D11Texture2D* ppTexture2;
	ID3D11Texture2D* ppTexture3;

	device->CreateTexture2D(&textureDesc, 0, &ppTexture);
	device->CreateTexture2D(&textureDesc, 0, &ppTexture2);
	device->CreateTexture2D(&textureDesc, 0, &ppTexture3);

	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	device->CreateRenderTargetView(ppTexture, &rtvDesc, &blurRTV);
	device->CreateRenderTargetView(ppTexture2, &rtvDesc, &blur2RTV);
	device->CreateRenderTargetView(ppTexture3, &rtvDesc, &finalRTV);

	// Create the Shader Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = textureDesc.Format;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;

	device->CreateShaderResourceView(ppTexture, &srvDesc, &blurSRV);
	device->CreateShaderResourceView(ppTexture2, &srvDesc, &blur2SRV);
	device->CreateShaderResourceView(ppTexture3, &srvDesc, &finalSRV);

	// We don't need the texture reference itself no mo'
	ppTexture->Release();
	ppTexture2->Release();


	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadMaterials();
	

	// Game Objects
	meshes.push_back(new Mesh(device, "../../Assets/Models/sphere.obj"));
	meshes.push_back(new Mesh(device, "../../Assets/Models/cube_inverted.obj"));
	meshes.push_back(new Mesh(device, "../../Assets/Models/helix.obj"));

	//GUI Mesh
	//meshes.push_back(new Mesh(device, "../../Assets/Models/cube.obj"));
	meshes.push_back(new Mesh(device, "../../Assets/Models/plane.obj"));

	// Gallery base
	meshes.push_back(new Mesh(device, "../../Assets/Models/gallery.obj"));


	exhibits.push_back(new Entity(meshes[0], materials[5], context));
	exhibits.push_back(new Entity(meshes[0], materials[0], context));
	exhibits[1]->SetPosition(XMFLOAT3(3, 0.25f, -3));
	exhibits[0]->SetPosition(XMFLOAT3(-3, 0.4f, -3));
	//entities.push_back(new Entity(meshes[1], materials[1], context));

	// Gallery base
	entities.push_back(new Entity(meshes[4], materials[3], context));
	entities[0]->SetPosition(XMFLOAT3(0.0f, -0.5f, 0.0f));
	entities[0]->SetScale(XMFLOAT3(2.5f, 2.5f, 2.5f));

	//UI Elements
	//Start Holder
	GUIElements.push_back(new Entity(meshes[3], starMaterials[0], context));
	GUIElements[0]->SetRotation(XMFLOAT3(-(3.141592654f / 2), 0, 0));
	GUIElements[0]->SetScale(XMFLOAT3(2.56f, .5f, 0.5f));

	//E to rate
	GUIElements.push_back(new Entity(meshes[3], materials[2], context));
	GUIElements[1]->SetRotation(XMFLOAT3(-(3.141592654f / 2), 0, 0));
	GUIElements[1]->SetScale(XMFLOAT3(389.0f / 300, 125.0f / 300, 125.0f / 300));

	//R to restart
	GUIElements.push_back(new Entity(meshes[3], materials[4], context));
	GUIElements[2]->SetRotation(XMFLOAT3(-(3.141592654f / 2), 0, 0));
	GUIElements[2]->SetScale(XMFLOAT3(449.0 / 400, 93.0f / 400, 93.0f / 400));


	// Define the world boundaries

	// Starting Room
	worldBounds.push_back(new BoundingBox(XMFLOAT3(-4.75f, 0.0f, -13.15f), XMFLOAT3(4.75f, 0.0f, -1.25f))); // Room
	worldBounds.push_back(new BoundingBox(XMFLOAT3(-1.45f, 0.0f, -2.00f), XMFLOAT3(1.45f, 0.0f, 2.00f))); // North Doorway
	worldBounds.push_back(new BoundingBox(XMFLOAT3(3.00f, 0.0f, -4.15f), XMFLOAT3(8.20f, 0.0f, -2.40f))); // East Doorway
	worldBounds.push_back(new BoundingBox(XMFLOAT3(3.00f, 0.0f, -12.00f), XMFLOAT3(8.20f, 0.0f, -10.25f))); // Southeast Doorway

	// North Room
	worldBounds.push_back(new BoundingBox(XMFLOAT3(-4.80f, 0.0f, 0.35f), XMFLOAT3(4.80f, 0.0f, 13.30f))); // Room
	worldBounds.push_back(new BoundingBox(XMFLOAT3(-8.20f, 0.0f, 5.95f), XMFLOAT3(-3.00f, 0.0f, 7.70f))); // West Doorway
	worldBounds.push_back(new BoundingBox(XMFLOAT3(3.00f, 0.0f, 5.95f), XMFLOAT3(8.20f, 0.0f, 7.70f))); // East Doorway
	
	// Northwest Room
	worldBounds.push_back(new BoundingBox(XMFLOAT3(-14.90f, 0.0f, 2.60f), XMFLOAT3(-6.40f, 0.0f, 11.10f)));

	// Northeast Room
	worldBounds.push_back(new BoundingBox(XMFLOAT3(6.45f, 0.0f, 2.60f), XMFLOAT3(14.95f, 0.0f, 11.10f))); // Room
	worldBounds.push_back(new BoundingBox(XMFLOAT3(9.80f, 0.0f, -1.20f), XMFLOAT3(11.55f, 0.0f, 4.35f))); // South Doorway

	// East Room
	worldBounds.push_back(new BoundingBox(XMFLOAT3(6.45f, 0.0f, -7.50f), XMFLOAT3(14.95f, 0.0f, 0.95f)));

	// Southeast Room
	worldBounds.push_back(new BoundingBox(XMFLOAT3(6.45f, 0.0f, -16.50f), XMFLOAT3(14.95f, 0.0f, -9.15f)));

	//make walls "visible" by adding an inverted box to represent their dimensions
	// entities[0]->SetScale(XMFLOAT3(worldBounds[0]->GetHalfSize().x + 0.2f, 3.0f, worldBounds[0]->GetHalfSize().z + 0.2f));
	// entities[0]->SetPosition(XMFLOAT3(worldBounds[0]->GetCenter().x, 1.0f, worldBounds[0]->GetCenter().z));

	//entities[2]->SetScale(XMFLOAT3(worldBounds[1]->GetHalfSize().x + 0.2f, 3.0f, worldBounds[0]->GetHalfSize().y + 0.2f));
	//entities[2]->SetPosition(XMFLOAT3(worldBounds[1]->GetCenter().x, 0.0f, worldBounds[0]->GetCenter().y));

	// Define the exhibit bounds
	exhibitBounds.push_back(new BoundingBox(XMFLOAT3(-4.0f, 0.0f, -4.0f), XMFLOAT3(-2.0f, 0.0f, -2.0f)));	// Checker Sphere
	exhibitBounds.push_back(new BoundingBox(XMFLOAT3(2.0f, 0.0f, -4.0f), XMFLOAT3(4.0f, 0.0f, -2.0f)));		// Lava Sphere

	light.AmbientColor = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	light.DiffuseColor = XMFLOAT4(1, 1, 1, 1);
	light.Direction = XMFLOAT3(1, -1, 0);

	fullBright.AmbientColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	fullBright.DiffuseColor = XMFLOAT4(1, 1, 1, 1);
	fullBright.Direction = XMFLOAT3(1, -1, 0);

	/*************************************************************************/
	// Create a rasterizer state to draw
	// both the inside and outside of our objects
	D3D11_RASTERIZER_DESC rd = {};
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_NONE; // Don't cull front or back!
	device->CreateRasterizerState(&rd, &rast);

	// Set the state
	context->RSSetState(rast);

	// Create a blend state that does basic alpha blending
	D3D11_BLEND_DESC bd = {};
	bd.AlphaToCoverageEnable = false;
	bd.IndependentBlendEnable = false;
	bd.RenderTarget[0].BlendEnable = true;
	bd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	bd.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	bd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	bd.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	bd.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	bd.RenderTarget[0].RenderTargetWriteMask =
		D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&bd, &blend);

	// Turn on the blend state
	float factors[] = { 1,1,1,1 };
	context->OMSetBlendState(blend, factors, 0xFFFFFFFF);
	/*************************************************************************/
	
	//Let's get that Sky Cube Map
	CreateDDSTextureFromFile(device, context, L"../../Assets/Textures/Sky/SunnyCubeMap.dds", 0, &skySRV);

	D3D11_RASTERIZER_DESC rs = {};
	rs.FillMode = D3D11_FILL_SOLID;
	rs.CullMode = D3D11_CULL_FRONT;
	rs.DepthClipEnable = true;
	device->CreateRasterizerState(&rs, &skyRasterizerState);

	D3D11_DEPTH_STENCIL_DESC ds = {};
	ds.DepthEnable = true;
	ds.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	ds.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&ds, &skyDepthState);

	skyVertexShader = new SimpleVertexShader(device, context);
	skyVertexShader->LoadShaderFile(L"SkyBoxVS.cso");

	skyPixelShader = new SimplePixelShader(device, context);
	skyPixelShader->LoadShaderFile(L"SkyBoxPS.cso");

	skyMesh = new Mesh(device, "../../Assets/Models/cube.obj");
	
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
	materials[0]->SetTexture(device, context, L"../../Assets/Textures/Diffuse/Lava_005_COLOR.jpg");
	materials[0]->SetSpecularMap(device, context, L"../../Assets/Textures/Specular/ALL_SPEC.png");
	materials[0]->SetNormalMap(device, context, L"../../Assets/Textures/Normal/Lava_005_NORM.jpg");
	materials[0]->GetVertexShader()->LoadShaderFile(L"VertexShader.cso");
	materials[0]->GetPixelShader()->LoadShaderFile(L"PixelShader.cso");

	// Panel Texture
	materials.push_back(new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context)));
	materials[1]->SetTexture(device, context, L"../../Assets/Textures/Specular/ALL_SPEC.png");
	materials[1]->SetSpecularMap(device, context, L"../../Assets/Textures/Specular/ALL_SPEC.png");
	materials[1]->SetNormalMap(device, context, L"../../Assets/Textures/Normal/panel_normal.png");
	materials[1]->GetVertexShader()->LoadShaderFile(L"VertexShader.cso");
	materials[1]->GetPixelShader()->LoadShaderFile(L"PixelShader.cso");

	//Rate Texture
	materials.push_back(new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context)));
	materials[2]->SetTexture(device, context, L"../../Assets/Textures/UI/rate.png");
	materials[2]->SetSpecularMap(device, context, L"../../Assets/Textures/Specular/ALL_SPEC.png");
	materials[2]->SetNormalMap(device, context, L"../../Assets/Textures/Normal/NO_NORMAL.jpg");
	materials[2]->GetVertexShader()->LoadShaderFile(L"VertexShader.cso");
	materials[2]->GetPixelShader()->LoadShaderFile(L"PixelShader.cso");

	//White material
	materials.push_back(new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context)));
	materials[3]->SetTexture(device, context, L"../../Assets/Textures/Diffuse/white.png");
	materials[3]->SetSpecularMap(device, context, L"../../Assets/Textures/Specular/ALL_SPEC.png");
	materials[3]->SetNormalMap(device, context, L"../../Assets/Textures/Normal/NO_NORMAL.jpg");
	materials[3]->GetVertexShader()->LoadShaderFile(L"VertexShader.cso");
	materials[3]->GetPixelShader()->LoadShaderFile(L"PixelShader.cso");

	//Restart Texture
	materials.push_back(new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context)));
	materials[4]->SetTexture(device, context, L"../../Assets/Textures/UI/restart.png");
	materials[4]->SetSpecularMap(device, context, L"../../Assets/Textures/Specular/ALL_SPEC.png");
	materials[4]->SetNormalMap(device, context, L"../../Assets/Textures/Normal/NO_NORMAL.jpg");
	materials[4]->GetVertexShader()->LoadShaderFile(L"VertexShader.cso");
	materials[4]->GetPixelShader()->LoadShaderFile(L"PixelShader.cso");
	
	//Tiles Texture
	materials.push_back(new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context)));
	materials[5]->SetTexture(device, context, L"../../Assets/Textures/Diffuse/tiles_diffuse.png");
	materials[5]->SetSpecularMap(device, context, L"../../Assets/Textures/Specular/tiles_spec.png");
	materials[5]->SetNormalMap(device, context, L"../../Assets/Textures/Normal/tiles_normal.png");
	materials[5]->GetVertexShader()->LoadShaderFile(L"VertexShader.cso");
	materials[5]->GetPixelShader()->LoadShaderFile(L"PixelShader.cso");

	//loop through all the ui star materials
	for (int i = 0; i < 6; i++) {
		starMaterials.push_back(new Material(new SimpleVertexShader(device, context), new SimplePixelShader(device, context)));

		//concatenate a wstring then reference its first index to get a wchar_t* object
		std::wstring w_file = L"../../Assets/Textures/UI/ui_starTray_";
		w_file += std::to_wstring(i);
		w_file += L".png";

		starMaterials[i]->SetTexture(device, context, &w_file[0]);
		starMaterials[i]->SetSpecularMap(device, context, L"../../Assets/Textures/Specular/NO_SPEC.png");
		starMaterials[i]->SetNormalMap(device, context, L"../../Assets/Textures/Normal/NO_NORMAL.jpg");
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
	exhibits[0]->SetRotation(XMFLOAT3(0, totalTime * 0.5f, 0));
	exhibits[1]->SetRotation(XMFLOAT3(0, totalTime * 0.5f, 0));

	GUIElements[0]->SetPosition(XMFLOAT3((float)width / (2 * 100), 0.73f, 2));
	GUIElements[1]->SetPosition(XMFLOAT3((float)width / (2 * 100), (float)height / (2 * 100), 2));
	GUIElements[2]->SetPosition(XMFLOAT3(1.5f, 0.5f, 2));

	// Movement
	XMFLOAT3 prevPosition = GameCamera->GetPosition();	// Position before the move
	GameCamera->Update(deltaTime);

	XMFLOAT3 newPosition = GameCamera->GetPosition();	// Position after the move

	// Calculate exhibit collisions
	for (int i = 0; i < exhibitBounds.size(); i++)
	{
		if (exhibitBounds[i]->PointInside(newPosition))
		{
			XMFLOAT3 edge = exhibitBounds[i]->VectorToEdgeFromInside(newPosition);

			newPosition.x += edge.x;
			newPosition.z += edge.z;
		}
	}

	// Calculate gallery bound collisions
	bool willBeInBox = false;
	static int cameraCurBox = 0;		// Static so that the game always knows the last box the player was in
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
	if (worldBounds.size() > 0 && !willBeInBox)
	{
		// Adjust the player's new position to the edge of their box
		XMFLOAT3 edge = worldBounds[cameraCurBox]->VectorToEdge(newPosition);

		newPosition.x += edge.x;
		newPosition.z += edge.z;
	}

	// Translate by delta position!
	GameCamera->TranslateTo(newPosition.x - prevPosition.x, 0.0f, newPosition.z - prevPosition.z);

	// Lerp to make collision detection less jank
	XMFLOAT3 newestPosition;
	XMStoreFloat3(&newestPosition, DirectX::XMVectorLerp(XMLoadFloat3(&prevPosition), XMLoadFloat3(&GameCamera->GetPosition()), .5f));

	GameCamera->SetPosition(newestPosition);
	
	DoExhibits();

	if (GetAsyncKeyState('R') & 0x8000) {
		for (int i = 0; i < exhibits.size(); i++) {
			exhibits[i]->SetRating(-1);
		}
		GameCamera->SetPosition(XMFLOAT3(0, 0, -5));
		GameCamera->SetRotation(GameCamera->GetInitRotation());
	}
}

//calculate stars for game rating system
void Game::DoStars()
{
	if (!isRating) return;
	starRating = -1;
	if (exhibits[currentExhibit]->GetRating() != -1) {
		return;
	}
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

void Game::DoExhibits()
{
	canRate = false;
	bool isNear = false;

	//cycle through exhibits and see if we're close to one
	for (int i = 0; i < exhibits.size(); i++) {
		float distance = sqrt(
			pow((GameCamera->GetPosition().x - exhibits[i]->GetPosition().x), 2)
			+ pow((GameCamera->GetPosition().z - exhibits[i]->GetPosition().z), 2));
		if (distance < 2.5f) {
			canRate = true;
			isNear = true;
			currentExhibit = i;
			if (!isRating) {
				int myRating = exhibits[i]->GetRating();
				if (myRating == -1) myRating = 0;
				GUIElements[0]->SetMaterial(starMaterials[myRating]);
			}
			DoStars();
		}
	}
	//if not near any exhibits we cannot rate by default
	if (!isNear) isRating = false;
	
	if (GetAsyncKeyState('E') & 0x8000 && canRate) {
		isRating = true;
		exhibits[currentExhibit]->SetRating(-1);
	}
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	context->OMSetRenderTargets(1, &blurRTV, depthStencilView);

	// Set buffers in the input assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	// Background color (Black in this case) for clearing
	const float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	// Clear the render target and depth buffer (erases what's on the screen)
	//  - Do this ONCE PER FRAME
	//  - At the beginning of Draw (before drawing *anything*)
	context->ClearRenderTargetView(blurRTV, color);
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

	for (int i = 0; i < exhibits.size(); i++) {
		exhibits[i]->GetMaterial()->GetPixelShader()->SetData("light", &light, sizeof(DirectionalLight));
		exhibits[i]->GetMaterial()->GetPixelShader()->SetFloat3("cameraPosition", GameCamera->GetPosition());
		exhibits[i]->Render(GameCamera->GetView(), GameCamera->GetProjection());
	}
	
	//----------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------
	//SKYYYYYYYYBOXXXXXX
	context->RSSetState(skyRasterizerState);
	context->OMSetDepthStencilState(skyDepthState, 0);

	// After drawing all of our regular (solid) objects, draw the sky!
	ID3D11Buffer* skyVB = skyMesh->GetVertexBuffer();
	ID3D11Buffer* skyIB = skyMesh->GetIndexBuffer();

	// Set the buffers
	context->IASetVertexBuffers(0, 1, &skyVB, &stride, &offset);
	context->IASetIndexBuffer(skyIB, DXGI_FORMAT_R32_UINT, 0);

	skyVertexShader->SetMatrix4x4("view", GameCamera->GetView());
	skyVertexShader->SetMatrix4x4("projection", GameCamera->GetProjection());

	skyVertexShader->CopyAllBufferData();
	skyVertexShader->SetShader();

	// Send texture-related stuff
	skyPixelShader->SetShaderResourceView("SkyTex", skySRV);
	skyPixelShader->SetSamplerState("SkySampler", sampleState);

	skyPixelShader->CopyAllBufferData(); // Remember to copy to the GPU!!!!
	skyPixelShader->SetShader();

	// Finally do the actual drawing
	context->DrawIndexed(skyMesh->GetIndexCount(), 0, 0);

	//Reset changed states
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);
	//----------------------------------------------------------------------------------
	//----------------------------------------------------------------------------------

	//backBufferRTV
	// Done with scene render - swap back to the back buffer
	context->OMSetRenderTargets(1, &blur2RTV, 0);

	// Post process draw ================================
	context->ClearRenderTargetView(blur2RTV, color);

	ppVS->SetShader();
	blurPS->SetShader();
	blurPS->SetInt("xDir", 0);
	blurPS->SetInt("blurAmount", 10);
	blurPS->SetFloat("pixelWidth", 1.0f / width);
	blurPS->SetFloat("pixelHeight", 1.0f / height);
	blurPS->CopyAllBufferData();

	blurPS->SetShaderResourceView("Pixels", blurSRV);
	blurPS->SetSamplerState("Sampler", sampleState);

	// Unbind vertex and index buffers!
	ID3D11Buffer* nothing = 0;
	context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	// Draw exactly 3 vertices
	context->Draw(3, 0);
	blurPS->SetShaderResourceView("Pixels", 0);

	
	// Done with scene render - swap back to the back buffer
	context->OMSetRenderTargets(1, &finalRTV, 0);

	// Post process draw ================================
	context->ClearRenderTargetView(finalRTV, color);

	blurPS->SetInt("xDir", 1);
	blurPS->CopyAllBufferData();

	blurPS->SetShaderResourceView("Pixels", blur2SRV);

	// Unbind vertex and index buffers!
	context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	// Draw exactly 3 vertices
	context->Draw(3, 0);
	// Now that we're done, UNBIND the srv from the pixel shader
	blurPS->SetShaderResourceView("Pixels", 0);

	/**********************************************************************/	

	// Done with scene render - swap back to the back buffer
	context->OMSetRenderTargets(1, &backBufferRTV, 0);

	// Post process draw ================================
	context->ClearRenderTargetView(backBufferRTV, color);

	addBlendPS->SetShader();
	addBlendPS->SetShaderResourceView("Original", blurSRV);
	addBlendPS->SetShaderResourceView("Pixels", finalSRV);
	addBlendPS->SetSamplerState("Sampler", sampleState);

	// Unbind vertex and index buffers!
	context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);
	context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);

	// Draw exactly 3 vertices
	context->Draw(3, 0);

	// Now that we're done, UNBIND the srv from the pixel shader
	blurPS->SetShaderResourceView("Pixels", 0);
	
	//reset depth buffer before rendering UI
	context->ClearDepthStencilView(
		depthStencilView,
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.0f,
		0);


	if (canRate) {
		//for (int i = 0; i < GUIElements.size(); i++) {
		GUIElements[0]->GetMaterial()->GetPixelShader()->SetData("light", &fullBright, sizeof(DirectionalLight));
		GUIElements[0]->GetMaterial()->GetPixelShader()->SetFloat3("cameraPosition", GUICamera->GetPosition());
		GUIElements[0]->Render(GUICamera->GetView(), GUICamera->GetProjection());
		//}
	}
	if (!isRating && canRate) {
		GUIElements[1]->GetMaterial()->GetPixelShader()->SetData("light", &fullBright, sizeof(DirectionalLight));
		GUIElements[1]->GetMaterial()->GetPixelShader()->SetFloat3("cameraPosition", GUICamera->GetPosition());
		GUIElements[1]->Render(GUICamera->GetView(), GUICamera->GetProjection());
	}
	GUIElements[2]->GetMaterial()->GetPixelShader()->SetData("light", &fullBright, sizeof(DirectionalLight));
	GUIElements[2]->GetMaterial()->GetPixelShader()->SetFloat3("cameraPosition", GUICamera->GetPosition());
	GUIElements[2]->Render(GUICamera->GetView(), GUICamera->GetProjection());

	// Reset any changed render states!
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);

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
	if (canRate && starRating != -1 && isRating) {
		exhibits[currentExhibit]->SetRating(starRating);
		GUIElements[0]->SetMaterial(starMaterials[starRating]);
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