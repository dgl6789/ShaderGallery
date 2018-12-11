#include "BoundingBox.h"

BoundingBox::BoundingBox(DirectX::XMFLOAT3 topLeft, DirectX::XMFLOAT3 bottomRight)
{
	this->halfSize.x = (bottomRight.x - topLeft.x) / 2.0f;
	this->halfSize.y = (bottomRight.y - topLeft.y) / 2.0f;
	this->halfSize.z = (bottomRight.z - topLeft.z) / 2.0f;

	this->center.x = topLeft.x + this->halfSize.x;
	this->center.y = topLeft.y + this->halfSize.y;
	this->center.z = topLeft.z + this->halfSize.z;
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
		&& point.z >= center.z - halfSize.z			// Point is further down than topmost boundary
		&& point.z <= center.z + halfSize.z;		// Point is further up than bottommost boundary
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
	if (point.z < center.z - halfSize.z)
	{
		response.z = center.z - halfSize.z - point.z;
	}

	// Are we below the box?
	else if (point.z > center.z + halfSize.z)
	{
		response.z = center.z + halfSize.z - point.z;
	}

	return response;
}

XMFLOAT3 BoundingBox::GetCenter()
{
	return center;
}

XMFLOAT3 BoundingBox::GetHalfSize()
{
	return halfSize;
}
