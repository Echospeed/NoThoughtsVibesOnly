// ============================================================================
// Collider.cpp - Collision Detection Helpers
// ============================================================================
// Provides overloaded isOverlapping() functions for:
//   - AABB vs AABB  (SquareCollider vs SquareCollider)
//   - Circle vs Circle (CircleCollider vs CircleCollider)
//   - AABB vs Mouse point (SquareCollider vs Mouse) - used for UI hit-testing
//
// All colliders use world-space coordinates.
// ============================================================================

#include "pch.hpp"
#include "Collider.hpp"

// ============================================================================
// isOverlapping - AABB vs AABB
// ============================================================================
// Standard Axis-Aligned Bounding Box overlap test.
// Returns true if the two rectangles share any area.
// ============================================================================
bool isOverlapping(const SquareCollider& A, const SquareCollider& B)
{
    const f32 halfWA = A.scale.x / 2.0f;
    const f32 halfHA = A.scale.y / 2.0f;
    const f32 leftA = A.position.x - halfWA;
    const f32 rightA = A.position.x + halfWA;
    const f32 topA = A.position.y + halfHA;
    const f32 botA = A.position.y - halfHA;

    const f32 halfWB = B.scale.x / 2.0f;
    const f32 halfHB = B.scale.y / 2.0f;
    const f32 leftB = B.position.x - halfWB;
    const f32 rightB = B.position.x + halfWB;
    const f32 topB = B.position.y + halfHB;
    const f32 botB = B.position.y - halfHB;

    // Separating axis test: no overlap if separated on either axis
    if (leftA > rightB || rightA < leftB) return false;
    if (topA  < botB || botA   > topB)  return false;

    return true;
}

// ============================================================================
// isOverlapping - Circle vs Circle
// ============================================================================
// Returns true if the distance between centres is less than the sum of radii.
// Uses squared distance to avoid a sqrt call.
// ============================================================================
bool isOverlapping(const CircleCollider& A, const CircleCollider& B)
{
    const f32 dx = A.position.x - B.position.x;
    const f32 dy = A.position.y - B.position.y;
    const f32 distSq = dx * dx + dy * dy;
    const f32 radiusSum = A.radius + B.radius;

    return distSq <= (radiusSum * radiusSum);
}

// ============================================================================
// isOverlapping - AABB vs Mouse point
// ============================================================================
// Point-in-rectangle test used for button hover detection.
// The mouse position is treated as an infinitely small point.
// ============================================================================
bool isOverlapping(const SquareCollider& A, const Mouse& B)
{
    const f32 halfW = A.scale.x / 2.0f;
    const f32 halfH = A.scale.y / 2.0f;
    const f32 left = A.position.x - halfW;
    const f32 right = A.position.x + halfW;
    const f32 bot = A.position.y - halfH;
    const f32 top = A.position.y + halfH;

    return (B.position.x >= left && B.position.x <= right &&
        B.position.y >= bot && B.position.y <= top);
}