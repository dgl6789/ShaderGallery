#pragma once
#include "Game.h"
#include "Vertex.h"

#include <fstream>

/// Mesh class defines a container for buffers which
/// define a discrete geometric body composed of Vertices.
class Mesh {
public:
	Mesh(ID3D11Device* pDevice, char* fileName);
	~Mesh();

	// Methods to set up the buffers this mesh needs to render.
	void CreateBuffers(Vertex* vertArray, int numVerts, unsigned int* indexArray, int numIndices, ID3D11Device* device);

	// Buffers to hold actual data
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	ID3D11Buffer* GetVertexBuffer() { return vertexBuffer; }
	ID3D11Buffer* GetIndexBuffer() { return indexBuffer; }
	int GetIndexCount() { return numIndices; }

	// Number of vertices in this mesh.
	// Used to calculate buffer byte width.
	int numVertices;
	int numIndices;

private:
	void CalculateTangents(Vertex * verts, int numVerts, int * indices, int numIndices);
	// DX Device
	ID3D11Device* device;
};