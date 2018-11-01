#include "Entity.h"

XMFLOAT4X4 Entity::GetWorldMatrix()
{
	if (dirty) UpdateWorldMatrix();
	
	return world;
}

void Entity::UpdateWorldMatrix() {
	// If the world matrix has changed, update it.
	if (dirty) {
		// Construct the new world matrix
		XMMATRIX w = XMMatrixScaling(scale.x, scale.y, scale.z) * // Scale matrix
			(XMMatrixRotationX(rotation.x) * XMMatrixRotationY(rotation.y) * XMMatrixRotationZ(rotation.z)) * // rotation matrix is composed of 3 rotation matrices
			XMMatrixTranslation(position.x, position.y, position.z); // Position matrix

		// Store the new world matrix
		XMStoreFloat4x4(&world, XMMatrixTranspose(w));

		// Reset whether the world matrix has changed.
		dirty = false;
	}
}

// Entity contructor sets default values
Entity::Entity(Mesh* pMesh, Material* pMaterial, ID3D11DeviceContext* pDeviceContext)
{
	// Initialize the world matrix
	XMMATRIX identity = XMMatrixIdentity();
	XMStoreFloat4x4(&world, identity);

	// Initialize transformation vectors
	position = XMFLOAT3(0, 0, 0);
	rotation = XMFLOAT3(0, 0, 0);
	scale = XMFLOAT3(1, 1, 1);

	// Set this entity's mesh
	mesh = pMesh;

	// Set this entity's material
	material = pMaterial;

	// Set the device context
	deviceContext = pDeviceContext;
}

// Entity destructor doesn't need to do anything --
// Mesh cleans up DX objects.
Entity::~Entity() { }

// Set this entity's position
void Entity::SetPosition(XMFLOAT3 pNewPosition) { 
	position = pNewPosition; 
	dirty = true; 
}

// Add to this entity's position
void Entity::OffsetPosition(XMFLOAT3 pOffset) { 
	position = XMFLOAT3(position.x + pOffset.x, position.y + pOffset.y, position.z + pOffset.z); 
	dirty = true;
}

void Entity::SetActive(bool pActive)
{
	active = pActive;
}

XMFLOAT3 Entity::GetPosition()
{
	return position;
}

XMFLOAT3 Entity::GetScale()
{
	return scale;
}

XMFLOAT3 Entity::GetRotation()
{
	return rotation;
}

// Set this entity's rotation (Euler)
void Entity::SetRotation(XMFLOAT3 pNewRotation) { 
	rotation = pNewRotation;
	dirty = true;
}

// Set this entity's scale
void Entity::SetScale(XMFLOAT3 pNewScale) {
	scale = pNewScale;
	dirty = true;
}

// Set the mesh this entity will render
void Entity::SetMesh(Mesh * pMesh) { mesh = pMesh; }

// Set the material the mesh will be rendered with
void Entity::SetMaterial(Material * pMaterial) { material = pMaterial; }

Material * Entity::GetMaterial()
{
	return material;
}

void Entity::PrepMaterial(XMFLOAT4X4 pView, XMFLOAT4X4 pProjection) {
	// Send data to shader variables
	//  - Do this ONCE PER OBJECT you're drawing
	//  - This is actually a complex process of copying data to a local buffer
	//    and then copying that entire buffer to the GPU.  
	//  - The "SimpleShader" class handles all of that for you.
	material->GetVertexShader()->SetMatrix4x4("world", GetWorldMatrix());
	material->GetVertexShader()->SetMatrix4x4("view", pView);
	material->GetVertexShader()->SetMatrix4x4("projection", pProjection);

	material->GetPixelShader()->SetSamplerState("basicSampler", material->GetSampleState());
	material->GetPixelShader()->SetShaderResourceView("diffuseTexture", material->GetSRV());


	// Once you've set all of the data you care to change for
	// the next draw call, you need to actually send it to the GPU
	//  - If you skip this, the "SetMatrix" calls above won't make it to the GPU!
	material->GetVertexShader()->CopyAllBufferData();
	material->GetPixelShader()->CopyAllBufferData();

	// Set the vertex and pixel shaders to use for the next Draw() command
	//  - These don't technically need to be set every frame...YET
	//  - Once you start applying different shaders to different objects,
	//    you'll need to swap the current shaders before each draw
	material->GetVertexShader()->SetShader();
	material->GetPixelShader()->SetShader();
	
}

// Render this entity.
void Entity::Render(XMFLOAT4X4 pView, XMFLOAT4X4 pProjection) {
	// Is this entity currently active?
	if (!active) return; 

	// Prepare the pixel/vertex shaders for rendering
	PrepMaterial(pView, pProjection);

	// If the entity's transform was changed since last render, 
	// Update it.
	if (dirty) UpdateWorldMatrix();

	// Render the mesh using the world matrix.
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	deviceContext->IASetVertexBuffers(0, 1, &mesh->vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(mesh->indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Finally do the actual drawing
	//  - Do this ONCE PER OBJECT you intend to draw
	//  - This will use all of the currently set DirectX "stuff" (shaders, buffers, etc)
	//  - DrawIndexed() uses the currently set INDEX BUFFER to look up corresponding
	//     vertices in the currently set VERTEX BUFFER
	deviceContext->DrawIndexed(
		mesh->numVertices,  // The number of indices to use (we could draw a subset if we wanted)
		0,						// Offset to the first index we want to use
		0);						// Offset to add to each index when looking up vertices
}
