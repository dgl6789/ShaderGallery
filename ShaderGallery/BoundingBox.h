#pragma once
#include <DirectXMath.h>

/*
NOTE:
	This class is currently made to work with 2D boxes, since that is what the museum will be defined by.
	It may be updated later to account for 3D boxes if it's deemed necessary.
*/
using namespace DirectX;
class BoundingBox
{
public:
	// Looking at the XZ plane, where +Z is up and +X is right
	BoundingBox(DirectX::XMFLOAT3 topLeft, DirectX::XMFLOAT3 bottomRight);
	~BoundingBox();

	bool PointInside(DirectX::XMFLOAT3 point);
	DirectX::XMFLOAT3 VectorToEdge(DirectX::XMFLOAT3 point);
	DirectX::XMFLOAT3 VectorToEdgeFromInside(DirectX::XMFLOAT3 point);
	XMFLOAT3 GetCenter();
	XMFLOAT3 GetHalfSize();

private:
	DirectX::XMFLOAT3 center;
	DirectX::XMFLOAT3 halfSize;
};