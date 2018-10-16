#pragma once
#include "Game.h"
#include "Vertex.h"

#include <fstream>

/// Mesh class defines a container for buffers which
/// define a discrete geometric body composed of Vertices.
class Mesh {
public:
	Mesh(ID3D11Device* pDevice, char* fileName);
	Mesh(ID3D11Device* pDevice, Vertex* vertices, int* indices, int pNumVertices);
	~Mesh();

	// Methods to set up the buffers this mesh needs to render.
	D3D11_BUFFER_DESC CreateBufferDescription(D3D11_BIND_FLAG pBindFlags, UINT pByteWidth);

	HRESULT InitVertexBuffer(D3D11_BUFFER_DESC pBufferDesc, Vertex* pVertexArray);
	HRESULT InitIndexBuffer(D3D11_BUFFER_DESC pBufferDesc, int* pIndexArray);

	// Buffers to hold actual data
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	// Number of vertices in this mesh.
	// Used to calculate buffer byte width.
	int numVertices;

private:
	// DX Device
	ID3D11Device* device;
};