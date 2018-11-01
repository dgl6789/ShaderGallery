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
	BoundingBox(DirectX::XMFLOAT2 center, DirectX::XMFLOAT2 halfSize);
	~BoundingBox();

	bool PointInside(DirectX::XMFLOAT3 point);
	DirectX::XMFLOAT3 VectorToEdge(DirectX::XMFLOAT3 point);
	XMFLOAT2 GetCenter();
	XMFLOAT2 GetHalfSize();

private:
	DirectX::XMFLOAT2 center;
	DirectX::XMFLOAT2 halfSize;
};