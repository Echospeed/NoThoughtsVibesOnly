// Author: Chia Wenjie
// Co-Author: John Chiow

// ============================================================================
// Transform.cpp - Transform Matrix Builder
// ============================================================================
// Builds Scale-Rotate-Translate (SRT) matrices for rendering.
// Apply() is the standard call - builds and immediately uploads to the GPU.
// GetMatrix() returns the matrix without uploading (for concatenation).
// TransformMovement() is kept for backward compatibility.
// ============================================================================

#include "pch.hpp"
#include "Transform.hpp"

// ============================================================================
// GetMatrix
// ============================================================================
// Builds the SRT matrix in order: Scale -> Rotate -> Translate.
// This matches standard game math convention (applied right-to-left in column
// major, but AEMtx33Concat handles ordering correctly).
// ============================================================================
AEMtx33 Transform::GetMatrix() const
{
    AEMtx33 sc, rot, tr, result;

    AEMtx33Scale(&sc, scale.x, scale.y);
    AEMtx33RotDeg(&rot, rotation);
    AEMtx33Trans(&tr, position.x, position.y);

    AEMtx33Concat(&result, &rot, &sc);   // Rotate * Scale
    AEMtx33Concat(&result, &tr, &result); // Translate * (Rotate * Scale)

    return result;
}

// ============================================================================
// Apply
// ============================================================================
// Builds the SRT matrix and uploads it to the GPU via AEGfxSetTransform.
// Call this just before AEGfxMeshDraw().
// ============================================================================
void Transform::Apply() const
{
    AEMtx33 m = GetMatrix();
    AEGfxSetTransform(m.m);
}

// ============================================================================
// TransformMovement (backward compatibility wrapper)
// ============================================================================
void TransformMovement(Transform& t)
{
    t.Apply();
}