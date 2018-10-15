#include "Mesh.h"

/// Mesh constructor takes arrays of vertices and indices,
/// and sets up a buffer for each.
Mesh::Mesh(ID3D11Device* pDevice, Vertex* vertices, int* indices, int pNumVertices) {
	// Initialize buffer fields
	vertexBuffer = 0;
	indexBuffer = 0;

	device = pDevice;
	numVertices = pNumVertices;

	InitIndexBuffer(CreateBufferDescription(D3D11_BIND_INDEX_BUFFER, numVertices * sizeof(int)), indices);
	InitVertexBuffer(CreateBufferDescription(D3D11_BIND_VERTEX_BUFFER, numVertices * sizeof(Vertex)), vertices);
}

/// Clean up the buffers
Mesh::~Mesh() {
	// Release any (and all!) DirectX objects we've made
	if (vertexBuffer) { vertexBuffer->Release(); }
	if (indexBuffer) { indexBuffer->Release(); }
}

/// Create the buffer description to use when initializing this mesh's buffer objects.
/// Returns the created buffer.
D3D11_BUFFER_DESC Mesh::CreateBufferDescription(D3D11_BIND_FLAG pBindFlags, UINT pByteWidth) {
	// Create the buffer description ------------------------------------
	// - The description is created on the stack because we only need
	//    it to create the buffer.  The description is then useless.
	D3D11_BUFFER_DESC bd;
	bd.Usage = D3D11_USAGE_IMMUTABLE;									
	bd.ByteWidth = pByteWidth;			// number of indices in the buffer
	bd.BindFlags = pBindFlags;							// Tells DirectX this is an buffer of the type specified in the params for this method.
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	bd.StructureByteStride = 0;

	return bd;
}

/// Initialize the vertex buffer.
/// Returns S_OK if successful. Saves off the buffer into &pVertexArray.
HRESULT Mesh::InitVertexBuffer(D3D11_BUFFER_DESC pBufferDesc, Vertex* pVertexArray) {
	// Create the proper struct to hold the initial vertex data
	// - This is how we put the initial data into the buffer
	D3D11_SUBRESOURCE_DATA lInitialVertexData;
	lInitialVertexData.pSysMem = pVertexArray;

	return device->CreateBuffer(&pBufferDesc, &lInitialVertexData, &vertexBuffer);
}

/// Initialize the vertex buffer.
/// Returns S_OK if successful. Saves off the buffer into &pIndexArray.
HRESULT Mesh::InitIndexBuffer(D3D11_BUFFER_DESC pBufferDesc, int* pIndexArray) {
	// Create the proper struct to hold the initial vertex data
	// - This is how we put the initial data into the buffer
	D3D11_SUBRESOURCE_DATA lIinitialIndexData;
	lIinitialIndexData.pSysMem = pIndexArray;

	return device->CreateBuffer(&pBufferDesc, &lIinitialIndexData, &indexBuffer);
}
