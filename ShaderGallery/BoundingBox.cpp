#include "BoundingBox.h"

BoundingBox::BoundingBox(DirectX::XMFLOAT2 center, DirectX::XMFLOAT2 halfSize)
{
	this->center = center;
	this->halfSize = halfSize;
}

BoundingBox::~BoundingBox()
{
}

// Return a bool saying if a point is inside of the box or not
bool BoundingBox::PointInside(DirectX::XMFLOAT3 point)
{
	return
		point.x >= center.x - halfSize.x			// Point is further right than leftmost boundary
		&& point.x <= center.x + halfSize.x			// Point is further left than rightmost boundary
		&& point.z >= center.y - halfSize.y			// Point is further down than topmost boundary
		&& point.z <= center.y + halfSize.y;		// Point is further up than bottommost boundary
}

// Return a vector pointing from a given point to the nearest edge of this box. Minimum distance.
DirectX::XMFLOAT3 BoundingBox::VectorToEdge(DirectX::XMFLOAT3 point)
{
	DirectX::XMFLOAT3 response = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);

	// Are we on the box's left?
	if (point.x < center.x - halfSize.x)
	{
		response.x = center.x - halfSize.x - point.x;
	}

	// Are we on the box's right?
	else if (point.x > center.x + halfSize.x)
	{
		response.x = center.x + halfSize.x - point.x;
	}

	// Are we above the box?
	if (point.z < center.y - halfSize.y)
	{
		response.z = center.y - halfSize.y - point.z;
	}

	// Are we below the box?
	else if (point.z > center.y + halfSize.y)
	{
		response.z = center.y + halfSize.y - point.z;
	}

	return response;
}

XMFLOAT2 BoundingBox::GetCenter()
{
	return center;
}

XMFLOAT2 BoundingBox::GetHalfSize()
{
	return halfSize;
}
