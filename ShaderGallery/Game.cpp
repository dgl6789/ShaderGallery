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

	XMStoreFloat4x4(
		&shadowViewMatrix,
		XMMatrixTranspose(XMMatrixLookAtLH(
			XMVectorSet(-20, 20, 0, 0),	// Position - Up 20 units and backwards 20 units
			XMVectorSet(0, 0, 0, 0),	// Target - Origin (0,0,0)
			XMVectorSet(0, 1, 0, 0)))); // Up - positive Y axis (0,1,0)

	XMStoreFloat4x4(
		&shadowProjectionMatrix,
		XMMatrixTranspose(XMMatrixOrthographicLH(
			10,			// Width of projection "box"
			10,			// Height of projection "box"
			0.1f,		// Near clip dist
			100.0f)));	// Far clip dist


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
	delete pixelShader;
	delete vertexShader;

	// Delete each added resource
	for (auto& m : meshes) delete m;
	for (auto& m : materials) delete m;
	for (auto& m : starMaterials) delete m;
	for (auto& e : entities) delete e;
	for (auto& e : exhibits) delete e;
	for (auto& e : emitters) delete e;
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

	// Clean up shadow map
	shadowDSV->Release();
	shadowSRV->Release();
	shadowRasterizer->Release();
	shadowSampler->Release();
	delete shadowVS;

	finalSRV->Release();
	finalRTV->Release();

	blurSRV->Release();
	blurRTV->Release();

	blur2SRV->Release();
	blur2RTV->Release();

	delete sampleDescription;
	sampleState->Release();

	delete particlePS;
	delete particleVS;
	particleBlendState->Release();
	particleDepthState->Release();
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	pixelShader = new SimplePixelShader(device, context);
	pixelShader->LoadShaderFile(L"PixelShader.cso");

	vertexShader = new SimpleVertexShader(device, context);
	vertexShader->LoadShaderFile(L"VertexShader.cso");

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

	// Load shadow map shader
	shadowVS = new SimpleVertexShader(device, context);
	shadowVS->LoadShaderFile(L"ShadowMapVS.cso");

	// Load particle shaders
	particlePS = new SimplePixelShader(device, context);
	particlePS->LoadShaderFile(L"ParticlePS.cso");
	particleVS = new SimpleVertexShader(device, context);
	particleVS->LoadShaderFile(L"ParticleVS.cso");

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
	ppTexture3->Release();


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

	// Exhibits
	meshes.push_back(new Mesh(device, "../../Assets/Models/painting_large.obj"));
	meshes.push_back(new Mesh(device, "../../Assets/Models/painting_small.obj"));
	meshes.push_back(new Mesh(device, "../../Assets/Models/wackybigsculpture.obj"));
	meshes.push_back(new Mesh(device, "../../Assets/Models/bench.obj"));
	meshes.push_back(new Mesh(device, "../../Assets/Models/painting_small_h.obj"));
	meshes.push_back(new Mesh(device, "../../Assets/Models/wackysculpture1.obj"));

	exhibits.push_back(new Entity(meshes[0], materials[5], context)); // tiles
	exhibits.push_back(new Entity(meshes[0], materials[0], context)); // lava
	exhibits.push_back(new Entity(meshes[5], materials[7], context)); // big painting
	exhibits.push_back(new Entity(meshes[7], materials[8], context)); // big sculpture

	exhibits.push_back(new Entity(meshes[9], materials[10], context)); // painting 1
	exhibits.push_back(new Entity(meshes[9], materials[11], context)); // painting 2
	exhibits.push_back(new Entity(meshes[5], materials[13], context)); // painting 3
	exhibits.push_back(new Entity(meshes[10], materials[14], context)); // weird donut

	exhibits[1]->SetPosition(XMFLOAT3(1, 0.75f, 5));
	exhibits[0]->SetPosition(XMFLOAT3(-8, 0.9f, 5));
	exhibits[1]->SetScale(XMFLOAT3(2, 2, 2));
	exhibits[0]->SetScale(XMFLOAT3(2, 2, 2));

	exhibits[2]->SetPosition(XMFLOAT3(-4, 2, 16));

	exhibits[3]->SetScale(XMFLOAT3(2, 2, 2));
	exhibits[3]->SetPosition(XMFLOAT3(-4, -1, -15));

	exhibits[4]->SetPosition(XMFLOAT3(-10.5f, 1.5f, -6)); // paintings
	exhibits[5]->SetRotation(XMFLOAT3(0, 3.14f, 0));
	exhibits[5]->SetPosition(XMFLOAT3(3, 1.5, -6));
	exhibits[6]->SetPosition(XMFLOAT3(10.5f, 2.5, -18));
	exhibits[6]->SetRotation(XMFLOAT3(0, 3.14f, 0));
	exhibits[7]->SetPosition(XMFLOAT3(-18.5f, 1, 2.5f));
	exhibits[7]->SetRotation(XMFLOAT3(0, 3.14f / 2, 0));

	// Gallery base
	entities.push_back(new Entity(meshes[4], materials[6], context));
	entities[0]->SetPosition(XMFLOAT3(0.0f, -0.5f, 0.0f));
	entities[0]->SetScale(XMFLOAT3(2.5f, 2.5f, 2.5f));

	entities.push_back(new Entity(meshes[8], materials[9], context));
	entities.push_back(new Entity(meshes[8], materials[9], context));
	entities.push_back(new Entity(meshes[8], materials[9], context));
	entities.push_back(new Entity(meshes[8], materials[9], context));
	entities[1]->SetPosition(XMFLOAT3(-1.5f, -0.65f, 5)); // Benches
	entities[1]->SetScale(XMFLOAT3(0.5f, 0.5f, 0.5f));
	entities[2]->SetPosition(XMFLOAT3(-6, -0.65f, 5));
	entities[2]->SetScale(XMFLOAT3(0.5f, 0.5f, 0.5f));
	entities[3]->SetPosition(XMFLOAT3(-4, -0.65f, 11));
	entities[3]->SetScale(XMFLOAT3(0.5f, 0.5f, 0.5f));
	entities[3]->SetRotation(XMFLOAT3(0, 3.14f / 2, 0));
	entities[4]->SetPosition(XMFLOAT3(10.5f, -0.65f, -13.5f));
	entities[4]->SetScale(XMFLOAT3(0.5f, 0.5f, 0.5f));
	entities[4]->SetRotation(XMFLOAT3(0, 3.14f / 2, 0));

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
	worldBounds.push_back(new BoundingBox(XMFLOAT3(-10, 0.0f, -18.3875f), XMFLOAT3(2.50f, 0.0f, 15.5f))); // Room
	worldBounds.push_back(new BoundingBox(XMFLOAT3(-12, 0.0f, 1), XMFLOAT3(-9.5f, 0.0f, 4))); // West Doorway
	worldBounds.push_back(new BoundingBox(XMFLOAT3(2, 0.0f, 1), XMFLOAT3(5, 0.0f, 4))); // East Doorway

	//// West Room
	worldBounds.push_back(new BoundingBox(XMFLOAT3(-24, 0.0f, -4.0f), XMFLOAT3(-12, 0.0f, 8))); // Room

	//// East Room
	worldBounds.push_back(new BoundingBox(XMFLOAT3(5, 0.0f, -3), XMFLOAT3(17, 0.0f, 9)));
	worldBounds.push_back(new BoundingBox(XMFLOAT3(10, 0.0f, -6), XMFLOAT3(11, 0.0f, -2))); // South door

	//// Southeast Room
	worldBounds.push_back(new BoundingBox(XMFLOAT3(5, 0.0f, -17), XMFLOAT3(17, 0.0f, -6))); // Room

	// Bounds testing block
	// entities.push_back(new Entity(meshes[1], materials[3], context));
	// entities[1]->SetScale(XMFLOAT3(worldBounds[6]->GetHalfSize().x, 1.0f, worldBounds[6]->GetHalfSize().z));
	// entities[1]->SetPosition(XMFLOAT3(worldBounds[6]->GetCenter().x, 1.0f, worldBounds[6]->GetCenter().z));

	// Define the exhibit bounds
	// exhibitBounds.push_back(new BoundingBox(XMFLOAT3(-4.0f, 0.0f, -4.0f), XMFLOAT3(-2.0f, 0.0f, -2.0f)));	// Checker Sphere
	// exhibitBounds.push_back(new BoundingBox(XMFLOAT3(2.0f, 0.0f, -4.0f), XMFLOAT3(4.0f, 0.0f, -2.0f)));		// Lava Sphere

	// Particle Emitters
	// A depth state for the particles
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO; // Turns off depth writing
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	device->CreateDepthStencilState(&dsDesc, &particleDepthState);


	// Blend for particles (additive)
	D3D11_BLEND_DESC particleBlend = {};
	particleBlend.AlphaToCoverageEnable = false;
	particleBlend.IndependentBlendEnable = false;
	particleBlend.RenderTarget[0].BlendEnable = true;
	particleBlend.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	particleBlend.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
	particleBlend.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	particleBlend.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	particleBlend.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	particleBlend.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	particleBlend.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device->CreateBlendState(&particleBlend, &particleBlendState);

	// Create Emitters
	emitters.push_back(new Emitter(
		XMFLOAT3(-18.5f, 2.75f, 2.75f),				// Position
		XMFLOAT3(0.1f, 0.003125f, 0.1f),			// Initial Particle Velocity
		XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),		// Initial Particle Color
		XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f),		// Final Particle Color
		1.25f,									// Initial Particle Size
		0.45f,									// Final Particle Size
		1000,									// Max Number of Particles
		20.0f,									// Particles per Second
		2.0f,									// Particle Lifetime
		device,
		materials[12]->GetVertexShader(),
		materials[12]->GetPixelShader(),
		materials[12]->GetTexture()
		));

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
	SetUpShadowMap();

	// Tell the input assembler stage of the pipeline what kind of
	// geometric primitives (points, lines or triangles) we want to draw.  
	// Essentially: "What kind of shape should the GPU draw with our data?"
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Game::SetUpShadowMap()
{
	// Create shadow requirements ------------------------------------------
	shadowMapSize = 1024;


	// Create the actual texture that will be the shadow map
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = shadowMapSize;
	shadowDesc.Height = shadowMapSize;
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	ID3D11Texture2D* shadowTexture;
	device->CreateTexture2D(&shadowDesc, 0, &shadowTexture);

	// Create the depth/stencil
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(shadowTexture, &shadowDSDesc, &shadowDSV);

	// Create the SRV for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(shadowTexture, &srvDesc, &shadowSRV);

	// Release the texture reference since we don't need it
	shadowTexture->Release();

	// Create the special "comparison" sampler state for shadows
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; // Could be anisotropic
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f;
	shadowSampDesc.BorderColor[1] = 1.0f;
	shadowSampDesc.BorderColor[2] = 1.0f;
	shadowSampDesc.BorderColor[3] = 1.0f;
	device->CreateSamplerState(&shadowSampDesc, &shadowSampler);

	// Create a rasterizer state
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Multiplied by (smallest possible value > 0 in depth buffer)
	shadowRastDesc.DepthBiasClamp = 0.0f;
	shadowRastDesc.SlopeScaledDepthBias = 1.0f;
	device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);
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
	materials.push_back(new Material(vertexShader, pixelShader));
	materials[0]->SetTexture(device, context, L"../../Assets/Textures/Diffuse/Lava_005_COLOR.jpg");
	materials[0]->SetSpecularMap(device, context, L"../../Assets/Textures/Specular/ALL_SPEC.png");
	materials[0]->SetNormalMap(device, context, L"../../Assets/Textures/Normal/Lava_005_NORM.jpg");

	// Panel Texture
	materials.push_back(new Material(vertexShader, pixelShader));
	materials[1]->SetTexture(device, context, L"../../Assets/Textures/Specular/ALL_SPEC.png");
	materials[1]->SetSpecularMap(device, context, L"../../Assets/Textures/Specular/ALL_SPEC.png");
	materials[1]->SetNormalMap(device, context, L"../../Assets/Textures/Normal/panel_normal.png");

	//Rate Texture
	materials.push_back(new Material(vertexShader, pixelShader));
	materials[2]->SetTexture(device, context, L"../../Assets/Textures/UI/rate.png");
	materials[2]->SetSpecularMap(device, context, L"../../Assets/Textures/Specular/ALL_SPEC.png");
	materials[2]->SetNormalMap(device, context, L"../../Assets/Textures/Normal/NO_NORMAL.jpg");

	//White material
	materials.push_back(new Material(vertexShader, pixelShader));
	materials[3]->SetTexture(device, context, L"../../Assets/Textures/Diffuse/white.png");
	materials[3]->SetSpecularMap(device, context, L"../../Assets/Textures/Specular/ALL_SPEC.png");
	materials[3]->SetNormalMap(device, context, L"../../Assets/Textures/Normal/NO_NORMAL.jpg");

	//Restart Texture
	materials.push_back(new Material(vertexShader, pixelShader));
	materials[4]->SetTexture(device, context, L"../../Assets/Textures/UI/restart.png");
	materials[4]->SetSpecularMap(device, context, L"../../Assets/Textures/Specular/ALL_SPEC.png");
	materials[4]->SetNormalMap(device, context, L"../../Assets/Textures/Normal/NO_NORMAL.jpg");
	
	//Tiles Texture
	materials.push_back(new Material(vertexShader, pixelShader));
	materials[5]->SetTexture(device, context, L"../../Assets/Textures/Diffuse/tiles_diffuse.png");
	materials[5]->SetSpecularMap(device, context, L"../../Assets/Textures/Specular/tiles_spec.png");
	materials[5]->SetNormalMap(device, context, L"../../Assets/Textures/Normal/tiles_normal.png");

	//Gallery Texture
	materials.push_back(new Material(vertexShader, pixelShader));
	materials[6]->SetTexture(device, context, L"../../Assets/Textures/Diffuse/galleryTexture.png");
	materials[6]->SetSpecularMap(device, context, L"../../Assets/Textures/Specular/ALL_SPEC.png");
	materials[6]->SetNormalMap(device, context, L"../../Assets/Textures/Normal/NO_NORMAL.jpg");

	materials.push_back(new Material(vertexShader, pixelShader));
	materials[7]->SetTexture(device, context, L"../../Assets/Textures/Diffuse/painting_0.png");
	materials[7]->SetSpecularMap(device, context, L"../../Assets/Textures/Specular/ALL_SPEC.png");
	materials[7]->SetNormalMap(device, context, L"../../Assets/Textures/Normal/painting_0_normal.png");

	materials.push_back(new Material(vertexShader, pixelShader));
	materials[8]->SetTexture(device, context, L"../../Assets/Textures/Diffuse/marble.png");
	materials[8]->SetSpecularMap(device, context, L"../../Assets/Textures/Specular/marbleSpecular.png");
	materials[8]->SetNormalMap(device, context, L"../../Assets/Textures/Normal/marbleNormal.png");

	materials.push_back(new Material(vertexShader, pixelShader));
	materials[9]->SetTexture(device, context, L"../../Assets/Textures/Diffuse/bench.png");
	materials[9]->SetSpecularMap(device, context, L"../../Assets/Textures/Specular/ALL_SPEC.png");
	materials[9]->SetNormalMap(device, context, L"../../Assets/Textures/Normal/NO_NORMAL.jpg");

	materials.push_back(new Material(vertexShader, pixelShader));
	materials[10]->SetTexture(device, context, L"../../Assets/Textures/Diffuse/painting_1.png");
	materials[10]->SetSpecularMap(device, context, L"../../Assets/Textures/Specular/ALL_SPEC.png");
	materials[10]->SetNormalMap(device, context, L"../../Assets/Textures/Normal/canvas_normal.png");

	materials.push_back(new Material(vertexShader, pixelShader));
	materials[11]->SetTexture(device, context, L"../../Assets/Textures/Diffuse/painting_2.png");
	materials[11]->SetSpecularMap(device, context, L"../../Assets/Textures/Specular/ALL_SPEC.png");
	materials[11]->SetNormalMap(device, context, L"../../Assets/Textures/Normal/canvas_normal.png");

	// Particle Texture
	materials.push_back(new Material(particleVS, particlePS));
	materials[12]->SetTexture(device, context, L"../../Assets/Textures/Particles/fireParticle.jpg");
	materials[12]->SetSpecularMap(device, context, L"../../Assets/Textures/Specular/NO_SPEC.png");
	materials[12]->SetNormalMap(device, context, L"../../Assets/Textures/Normal/NO_NORMAL.jpg");

	materials.push_back(new Material(vertexShader, pixelShader));
	materials[13]->SetTexture(device, context, L"../../Assets/Textures/Diffuse/painting_3.png");
	materials[13]->SetSpecularMap(device, context, L"../../Assets/Textures/Specular/painting_3_specular.png");
	materials[13]->SetNormalMap(device, context, L"../../Assets/Textures/Normal/NO_NORMAL.jpg");

	materials.push_back(new Material(vertexShader, pixelShader));
	materials[14]->SetTexture(device, context, L"../../Assets/Textures/Diffuse/volcanic.png");
	materials[14]->SetSpecularMap(device, context, L"../../Assets/Textures/Specular/ALL_SPEC.png");
	materials[14]->SetNormalMap(device, context, L"../../Assets/Textures/Normal/volcanic_normal.png");


	//loop through all the ui star materials
	for (int i = 0; i < 6; i++) {
		starMaterials.push_back(new Material(vertexShader, pixelShader));

		//concatenate a wstring then reference its first index to get a wchar_t* object
		std::wstring w_file = L"../../Assets/Textures/UI/ui_starTray_";
		w_file += std::to_wstring(i);
		w_file += L".png";

		starMaterials[i]->SetTexture(device, context, &w_file[0]);
		starMaterials[i]->SetSpecularMap(device, context, L"../../Assets/Textures/Specular/NO_SPEC.png");
		starMaterials[i]->SetNormalMap(device, context, L"../../Assets/Textures/Normal/NO_NORMAL.jpg");
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

	DoEmitters(deltaTime);

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

void Game::DoEmitters(float deltaTime)
{
	for (int i = 0; i < emitters.size(); i++)
	{
		emitters[i]->Update(deltaTime);
	}
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	DrawShadowMap();

	// Background color (Black in this case) for clearing
	const float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

	context->OMSetRenderTargets(1, &blurRTV, depthStencilView);

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
		entities[i]->GetMaterial()->GetPixelShader()->SetInt("ReceiveShadows", 1);
		entities[i]->GetMaterial()->GetPixelShader()->SetData("light", &light, sizeof(DirectionalLight));
		entities[i]->GetMaterial()->GetPixelShader()->SetFloat3("cameraPosition", GameCamera->GetPosition());
		entities[i]->GetMaterial()->GetVertexShader()->SetMatrix4x4("lightView", shadowViewMatrix);
		entities[i]->GetMaterial()->GetVertexShader()->SetMatrix4x4("lightProj", shadowProjectionMatrix);
		entities[i]->GetMaterial()->GetPixelShader()->SetShaderResourceView("ShadowMap", shadowSRV);
		entities[i]->GetMaterial()->GetPixelShader()->SetSamplerState("ShadowSampler", shadowSampler);

		entities[i]->Render(GameCamera->GetView(), GameCamera->GetProjection());
	}

	for (int i = 0; i < exhibits.size(); i++) {
		exhibits[i]->GetMaterial()->GetPixelShader()->SetInt("ReceiveShadows", 0);
		exhibits[i]->GetMaterial()->GetPixelShader()->SetData("light", &light, sizeof(DirectionalLight));
		exhibits[i]->GetMaterial()->GetPixelShader()->SetFloat3("cameraPosition", GameCamera->GetPosition());
		exhibits[i]->Render(GameCamera->GetView(), GameCamera->GetProjection());
	}

	// Particle states
	float particleBlend[4] = { 1,1,1,1 };
	context->OMSetBlendState(particleBlendState, particleBlend, 0xffffffff);  // Additive blending
	context->OMSetDepthStencilState(particleDepthState, 0);			// No depth WRITING

	for (int i = 0; i < emitters.size(); i++)
	{
		emitters[i]->Draw(context, GameCamera);
	}

	// Reset to default states
	context->OMSetBlendState(blend, particleBlend, 0xFFFFFFFF);

	DrawBloom();

	DrawUI();

	// Reset any states we've changed for the next frame!
	context->RSSetState(0);
	context->OMSetDepthStencilState(0, 0);

	// Present the back buffer to the user
	//  - Puts the final frame we're drawing into the window so the user can see it
	//  - Do this exactly ONCE PER FRAME (always at the very end of the frame)
	swapChain->Present(0, 0);
}

void Game::DrawShadowMap()
{
	// Initial setup of targets and states ============
	// No render target (we don't need colors), and set our shadow depth view
	context->OMSetRenderTargets(0, 0, shadowDSV);

	// Clear depth only
	context->ClearDepthStencilView(shadowDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

	// Special rasterizer to handle some shadow-map issues
	context->RSSetState(shadowRasterizer);


	// Need a viewport ===============
	// Must match the exact dimensions of our render target and/or depth buffer
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)shadowMapSize;
	viewport.Height = (float)shadowMapSize;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	// Set up shaders ============
	// This VS stuff is the same for every entity
	shadowVS->SetShader(); // Don't copy yet
	shadowVS->SetMatrix4x4("view", shadowViewMatrix);
	shadowVS->SetMatrix4x4("projection", shadowProjectionMatrix);

	// Turn OFF the pixel shader entirely,
	// since we don't need color output!
	context->PSSetShader(0, 0, 0);

	// Draw each entity ===================
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	for (unsigned int i = 0; i < exhibits.size(); i++)
	{
		// Grab the data from the first entity's mesh
		Entity* ge = exhibits[i];
		ID3D11Buffer* vb = ge->GetMesh()->GetVertexBuffer();
		ID3D11Buffer* ib = ge->GetMesh()->GetIndexBuffer();

		// Set buffers in the input assembler
		context->IASetVertexBuffers(0, 1, &vb, &stride, &offset);
		context->IASetIndexBuffer(ib, DXGI_FORMAT_R32_UINT, 0);

		// Copy entity-specific stuff to simple shader, and then to the GPU
		// (This could be optimized slightly by having two different constant buffers)
		shadowVS->SetMatrix4x4("world", ge->GetWorldMatrix());
		shadowVS->CopyAllBufferData();

		// Finally do the actual drawing
		context->DrawIndexed(ge->GetMesh()->numVertices, 0, 0);///////////////////////////////////////////////////////////////////////////
	}

	// Reset back to "regular" rendering options/targets ===========
	context->OMSetRenderTargets(1, &backBufferRTV, depthStencilView);
	viewport.Width = (float)this->width;
	viewport.Height = (float)this->height;
	context->RSSetViewports(1, &viewport); // Viewport that matches screen size
	context->RSSetState(0); // Default rasterizer options
}

void Game::DrawBloom()
{
	// Set buffers in the input assembler
	UINT stride = sizeof(Vertex);
	UINT offset = 0;

	// Background color (Black in this case) for clearing
	const float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

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
}

void Game::DrawUI()
{
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