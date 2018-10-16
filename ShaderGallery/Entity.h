#pragma once

#include <DirectXMath.h>
#include "Mesh.h"
#include "Material.h"

// For the DirectX Math library
using namespace DirectX;

class Mesh;
class Material;

class Entity
{
public:
	XMFLOAT4X4 GetWorldMatrix(); // Returns the world matrix of this entity
	void UpdateWorldMatrix(); // Update the world matrix of this entity

	Entity(Mesh* pMesh, Material* pMaterial, ID3D11DeviceContext* pDeviceContext);
	~Entity();

	void SetPosition(XMFLOAT3 pNewPosition); // Set position
	void OffsetPosition(XMFLOAT3 pOffset); // Add to current position

	void SetRotation(XMFLOAT3 pNewRotation); // Set rotation (Euler)
	void SetScale(XMFLOAT3 pNewScale); // Set scale

	void SetMesh(Mesh* pMesh); // Change/set mesh to render

	void SetMaterial(Material* pMaterial); // Change/set the material to render

	Material* GetMaterial(); // Get material

	void PrepMaterial(XMFLOAT4X4 pView, XMFLOAT4X4 pProjection); // Set data for drawing

	void Render(XMFLOAT4X4 pView, XMFLOAT4X4 pProjection); // Render the entity

private:
	bool dirty; // Does this entity's world matrix need to be updated?

	ID3D11DeviceContext* deviceContext; // Rendering context

	XMFLOAT4X4 world; // This entity's world matrix

	XMFLOAT3 position; // This entity's position vector
	XMFLOAT3 rotation; // This entity's rotation vector
	XMFLOAT3 scale;    // This entity's scale vector

	Mesh* mesh; // The mesh that this entity renders
	Material* material; // The material that the mesh is rendered with
};

