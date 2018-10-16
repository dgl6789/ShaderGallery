#include "Mesh.h"

Mesh::Mesh(ID3D11Device * pDevice, char * fileName)
{
	// File input object
	std::ifstream obj(fileName);

	// Check for successful open
	if (!obj.is_open())
		return;

	// Variables used while reading the file
	std::vector<XMFLOAT3> positions;     // Positions from the file
	std::vector<XMFLOAT3> normals;       // Normals from the file
	std::vector<XMFLOAT2> uvs;           // UVs from the file
	std::vector<Vertex> verts;           // Verts we're assembling
	std::vector<int> indices;           // Indices of these verts
	unsigned int vertCounter = 0;        // Count of vertices/indices
	char chars[100];                     // String for line reading

										 // Still have data left?
	while (obj.good())
	{
		// Get the line (100 characters should be more than enough)
		obj.getline(chars, 100);

		// Check the type of line
		if (chars[0] == 'v' && chars[1] == 'n')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			XMFLOAT3 norm;
			sscanf_s(
				chars,
				"vn %f %f %f",
				&norm.x, &norm.y, &norm.z);

			// Add to the list of normals
			normals.push_back(norm);
		}
		else if (chars[0] == 'v' && chars[1] == 't')
		{
			// Read the 2 numbers directly into an XMFLOAT2
			XMFLOAT2 uv;
			sscanf_s(
				chars,
				"vt %f %f",
				&uv.x, &uv.y);

			// Add to the list of uv's
			uvs.push_back(uv);
		}
		else if (chars[0] == 'v')
		{
			// Read the 3 numbers directly into an XMFLOAT3
			XMFLOAT3 pos;
			sscanf_s(
				chars,
				"v %f %f %f",
				&pos.x, &pos.y, &pos.z);

			// Add to the positions
			positions.push_back(pos);
		}
		else if (chars[0] == 'f')
		{
			// Read the face indices into an array
			unsigned int i[12];
			int facesRead = sscanf_s(
				chars,
				"f %d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d",
				&i[0], &i[1], &i[2],
				&i[3], &i[4], &i[5],
				&i[6], &i[7], &i[8],
				&i[9], &i[10], &i[11]);

			// - Create the verts by looking up
			//    corresponding data from vectors
			// - OBJ File indices are 1-based, so
			//    they need to be adusted
			Vertex v1;
			v1.Position = positions[i[0] - 1];
			v1.UV = uvs[i[1] - 1];
			v1.Normal = normals[i[2] - 1];

			Vertex v2;
			v2.Position = positions[i[3] - 1];
			v2.UV = uvs[i[4] - 1];
			v2.Normal = normals[i[5] - 1];

			Vertex v3;
			v3.Position = positions[i[6] - 1];
			v3.UV = uvs[i[7] - 1];
			v3.Normal = normals[i[8] - 1];

			// The model is most likely in a right-handed space,
			// especially if it came from Maya.  We want to convert
			// to a left-handed space for DirectX.  This means we 
			// need to:
			//  - Invert the Z position
			//  - Invert the normal's Z
			//  - Flip the winding order
			// We also need to flip the UV coordinate since DirectX
			// defines (0,0) as the top left of the texture, and many
			// 3D modeling packages use the bottom left as (0,0)

			// Flip the UV's since they're probably "upside down"
			v1.UV.y = 1.0f - v1.UV.y;
			v2.UV.y = 1.0f - v2.UV.y;
			v3.UV.y = 1.0f - v3.UV.y;

			// Flip Z (LH vs. RH)
			v1.Position.z *= -1.0f;
			v2.Position.z *= -1.0f;
			v3.Position.z *= -1.0f;

			// Flip normal Z
			v1.Normal.z *= -1.0f;
			v2.Normal.z *= -1.0f;
			v3.Normal.z *= -1.0f;

			// Add the verts to the vector (flipping the winding order)
			verts.push_back(v1);
			verts.push_back(v3);
			verts.push_back(v2);

			// Add three more indices
			indices.push_back(vertCounter); vertCounter += 1;
			indices.push_back(vertCounter); vertCounter += 1;
			indices.push_back(vertCounter); vertCounter += 1;

			// Was there a 4th face?
			if (facesRead == 12)
			{
				// Make the last vertex
				Vertex v4;
				v4.Position = positions[i[9] - 1];
				v4.UV = uvs[i[10] - 1];
				v4.Normal = normals[i[11] - 1];

				// Flip the UV, Z pos and normal
				v4.UV.y = 1.0f - v4.UV.y;
				v4.Position.z *= -1.0f;
				v4.Normal.z *= -1.0f;

				// Add a whole triangle (flipping the winding order)
				verts.push_back(v1);
				verts.push_back(v4);
				verts.push_back(v3);

				// Add three more indices
				indices.push_back(vertCounter); vertCounter += 1;
				indices.push_back(vertCounter); vertCounter += 1;
				indices.push_back(vertCounter); vertCounter += 1;
			}
		}
	}

	// Close the file and create the actual buffers
	obj.close();

	vertexBuffer = 0;
	indexBuffer = 0;

	device = pDevice;
	numVertices = vertCounter;

	InitIndexBuffer(CreateBufferDescription(D3D11_BIND_INDEX_BUFFER, numVertices * sizeof(int)), &indices[0]);
	InitVertexBuffer(CreateBufferDescription(D3D11_BIND_VERTEX_BUFFER, numVertices * sizeof(Vertex)), &verts[0]);
}

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
