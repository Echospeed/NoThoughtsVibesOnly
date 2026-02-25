#pragma once
#include "AEEngine.h"

struct Transform
{
    AEVec2 position{};
    AEVec2 scale{};
    f32    rotation{};

    // Build and apply the transform matrix in one call
    void Apply() const;

    // Build matrix without applying (useful for concatenation)
    AEMtx33 GetMatrix() const;

    // Quick setters
    void SetPosition(f32 x, f32 y) { position.x = x; position.y = y; }
    void SetScale(f32 x, f32 y) { scale.x = x;    scale.y = y; }
    void SetUniformScale(f32 s) { scale.x = s;    scale.y = s; }
};

void TransformMovement(Transform& t); // kept for backward compatibility