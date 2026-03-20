// ============================================================================
// Util.cpp - Mesh Creation and Rendering Helpers
// ============================================================================
// Provides:
//   Meshes namespace : Pre-built vertex lists (square, circle, triangle).
//   CreateSquare()   : Immediate-mode square draw with position/scale/colour.
//   CreateCircle()   : Immediate-mode circle draw.
//
// MESH LIFETIME:
// ----------------------------------------------------------------------------
//   Create in Load() with e.g. Meshes::CreateSquareCenterOriginMesh().
//   Free  in Unload() with Meshes::FreeMeshes().
//   Do NOT free meshes in Free() - they are shared across Init/Free cycles.
// ============================================================================

#include "pch.hpp"
#include "Util.hpp"

namespace Meshes
{
    // Global mesh handles - initialised by Create*() functions, freed by FreeMeshes()
    AEGfxVertexList* pSquareCOriMesh = nullptr;  // Center-origin square (most common)
    AEGfxVertexList* pSquareLOriMesh = nullptr;  // Left-origin square (for health bars etc.)
    AEGfxVertexList* pCircleMesh = nullptr;  // Circle (NPCs, AoE indicator)
    AEGfxVertexList* pTriangleMesh = nullptr;  // Triangle (melee NPC)

    // ========================================================================
    // CreateSquareCenterOriginMesh
    // ========================================================================
    // Two-triangle quad, centered at origin ([-0.5, 0.5] in each axis).
    // Scale in transform maps directly to world-unit dimensions.
    // ========================================================================
    void CreateSquareCenterOriginMesh()
    {
        AEGfxMeshStart();

        AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
            0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
            -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);

        AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
            0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
            -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);

        pSquareCOriMesh = AEGfxMeshEnd();
    }

    // ========================================================================
    // CreateSquareLeftOriginMesh
    // ========================================================================
    // Two-triangle quad, origin at the left edge ([0, 1] in X, [0, 1] in Y).
    // Useful for health bars that grow to the right from a fixed left edge.
    // ========================================================================
    void CreateSquareLeftOriginMesh()
    {
        AEGfxMeshStart();

        AEGfxTriAdd(0.0f, 0.0f, 0xFFFFFFFF, 0.0f, 1.0f,
            1.0f, 0.0f, 0xFFFFFFFF, 1.0f, 1.0f,
            0.0f, 1.0f, 0xFFFFFFFF, 0.0f, 0.0f);

        AEGfxTriAdd(1.0f, 0.0f, 0xFFFFFFFF, 1.0f, 1.0f,
            1.0f, 1.0f, 0xFFFFFFFF, 1.0f, 0.0f,
            0.0f, 1.0f, 0xFFFFFFFF, 0.0f, 0.0f);

        pSquareLOriMesh = AEGfxMeshEnd();
    }

    // ========================================================================
    // CreateCircleMesh
    // ========================================================================
    // 32-segment triangle-fan circle, centered at origin with radius 1.
    // Scale in transform controls the world-unit radius.
    // ========================================================================
    void CreateCircleMesh()
    {
        AEGfxMeshStart();

        const int steps = 32;
        const f32 angleStep = (2.0f * 3.14159f) / static_cast<f32>(steps);

        for (int i = 0; i < steps; ++i)
        {
            const f32 t1 = i * angleStep;
            const f32 t2 = (i + 1) * angleStep;

            AEGfxTriAdd(0.0f, 0.0f, 0xFFFFFFFF, 0.0f, 0.0f,
                cosf(t1), sinf(t1), 0xFFFFFFFF, 0.0f, 0.0f,
                cosf(t2), sinf(t2), 0xFFFFFFFF, 0.0f, 0.0f);
        }

        pCircleMesh = AEGfxMeshEnd();
    }

    // ========================================================================
    // CreateTriangleMesh
    // ========================================================================
    // Single upward-pointing triangle. Used for melee NPC appearance.
    // ========================================================================
    void CreateTriangleMesh()
    {
        AEGfxMeshStart();
        AEGfxTriAdd(0.0f, 1.0f, 0xFFFFFFFF, 0.0f, 0.0f,
            -1.0f, -1.0f, 0xFFFFFFFF, 0.0f, 0.0f,
            1.0f, -1.0f, 0xFFFFFFFF, 0.0f, 0.0f);
        pTriangleMesh = AEGfxMeshEnd();
    }

    // ========================================================================
    // FreeMeshes
    // ========================================================================
    // Frees all mesh vertex lists. Call in each state's Unload() function.
    // Guards against double-free with nullptr checks.
    // ========================================================================
    void FreeMeshes()
    {
        if (pSquareCOriMesh) { AEGfxMeshFree(pSquareCOriMesh); pSquareCOriMesh = nullptr; }
        if (pSquareLOriMesh) { AEGfxMeshFree(pSquareLOriMesh); pSquareLOriMesh = nullptr; }
        if (pCircleMesh) { AEGfxMeshFree(pCircleMesh);     pCircleMesh = nullptr; }
        if (pTriangleMesh) { AEGfxMeshFree(pTriangleMesh);   pTriangleMesh = nullptr; }
    }

} // namespace Meshes

// ============================================================================
// CreateSquare - Immediate draw helper
// ============================================================================
// Builds a transform from the given parameters and draws the mesh immediately.
// Use for one-off shapes (minimap dots, UI overlays) without a GameObject.
// ============================================================================
void CreateSquare(AEGfxVertexList* mesh, AEMtx33* transform,
    AEMtx33* scale, AEMtx33* rot, AEMtx33* trans,
    f32 xpos, f32 ypos, f32 scaleX, f32 scaleY, f32 rotation,
    f32 r, f32 g, f32 b, f32 a)
{
    AEGfxSetColorToMultiply(r, g, b, a);

    AEMtx33Scale(scale, scaleX, scaleY);
    AEMtx33Rot(rot, rotation);
    AEMtx33Trans(trans, xpos, ypos);

    AEMtx33Concat(transform, rot, scale);     // Rotate * Scale
    AEMtx33Concat(transform, trans, transform); // Translate * (R*S)

    AEGfxSetTransform(transform->m);
    AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
}

// ============================================================================
// CreateCircle - Immediate draw helper
// ============================================================================
void CreateCircle(AEGfxVertexList* mesh, AEMtx33* transform,
    AEMtx33* scale, AEMtx33* rot, AEMtx33* trans,
    f32 xpos, f32 ypos, f32 radius, f32 rotation,
    f32 r, f32 g, f32 b, f32 a)
{
    AEGfxSetColorToMultiply(r, g, b, a);

    AEMtx33Scale(scale, radius, radius);
    AEMtx33Rot(rot, rotation);
    AEMtx33Trans(trans, xpos, ypos);

    AEMtx33Concat(transform, rot, scale);
    AEMtx33Concat(transform, trans, transform);

    AEGfxSetTransform(transform->m);
    AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES);
}