#include "Collider.h"

bool isOverlapping(SquareCollider const& A, SquareCollider const& B)
{
	//AABB vs AABB collision detection
	// 1. Get the corners of the square A
	float halfWidthA = A.scale.x / 2.0f;
	float halfHeightA = A.scale.y / 2.0f;
	float leftA = A.position.x - halfWidthA;
	float rightA = A.position.x + halfWidthA;
	float topA = A.position.y + halfHeightA;
	float bottomA = A.position.y - halfHeightA;

	// 2. Get the corners of the transformed square B For simplicity, we will treat B as an AABB for now
	float halfWidthB = B.scale.x / 2.0f;
	float halfHeightB = B.scale.y / 2.0f;
	float leftB = B.position.x - halfWidthB;
	float rightB = B.position.x + halfWidthB;
	float topB = B.position.y + halfHeightB;
	float bottomB = B.position.y - halfHeightB;

	// 3. Check for overlap
	if (leftA > rightB || rightA < leftB || topA < bottomB || bottomA > topB) {
		return false; // No collision
	}
	return true; // Collision detected	
}

bool isOverlapping(CircleCollider const& A, CircleCollider const& B)
{
	// Circle vs Circle collision detection
	AEVec2 vecA{A.position.x, A.position.y};
	AEVec2 vecB{B.position.x, B.position.y};
	AEVec2 d{};

	AEVec2Sub(&d, &vecA, &vecB);
	
	f32 distanceSquared = d.x * d.x + d.y * d.y;
	f32 radiusSum = A.radius + B.radius;
	return distanceSquared <= (radiusSum * radiusSum);
}

bool isOverlapping(SquareCollider const& A, Mouse const& B)
{
	// AABB vs Mouse position collision detection
	f32 halfWidth = A.scale.x * 0.5f;
	f32 halfHeight = A.scale.y * 0.5f;

	// Compute AABB bounds
	f32 left = A.position.x - halfWidth;
	f32 right = A.position.x + halfWidth;
	f32 bottom = A.position.y - halfHeight;
	f32 top = A.position.y + halfHeight;

	// Point vs AABB check
	if (B.position.x < left || B.position.x > right ||
		B.position.y < bottom || B.position.y > top)
	{
		return false;
	}

	return true;
	
}