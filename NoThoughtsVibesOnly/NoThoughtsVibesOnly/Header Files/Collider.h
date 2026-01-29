#pragma once
#include "AEEngine.h"
#include "Input.h"

enum ColliderType {
	SQUARE,
	CIRCLE
};

struct SquareCollider {
	AEVec2 position;
	AEVec2 scale;
};

struct CircleCollider {
	AEVec2 position;
	f32 radius;
};

// Different Overloading for isColliding function
bool isOverlapping(SquareCollider const& A, SquareCollider const& B);
bool isOverlapping(CircleCollider const& A, CircleCollider const& B);
bool isOverlapping(SquareCollider const& A, Mouse const& B);
