// Author: Chia Wenjie
// Co-Author: John Chiow

// ============================================================================
// SpriteRenderer.cpp - Sprite and Shape Rendering
// ============================================================================
// Handles per-frame rendering of game entities using Alpha Engine's graphics API.
//
// TWO RENDERING MODES:
// ----------------------------------------------------------------------------
//   TEXTURE mode : Used when spriteRenderer.texture != nullptr.
//                  The texture is drawn as-is with the colour.a as alpha.
//   COLOR mode   : Used when no texture is set (most game objects).
//                  The colour RGBA is applied via SetColorToAdd.
//
// MESH TYPES:
// ----------------------------------------------------------------------------
//   MESH_SQUARE   -> Meshes::pSquareCOriMesh  (center-origin square)
//   MESH_TRIANGLE -> Meshes::pTriangleMesh
//   MESH_CIRCLE   -> Meshes::pCircleMesh
// ============================================================================

#include "pch.hpp"
#include "AEEngine.h"
#include "Util.hpp"
#include "SpriteRenderer.hpp"

// ============================================================================
// InitSpriteRenderer
// ============================================================================
// Loads a texture (or sets nullptr for colour-only rendering) and stores
// the mesh type and dimensions in the renderer struct.
// ============================================================================
void InitSpriteRenderer(SpriteRenderer& sr, const char* texturePath,
    f32 width, f32 height, MeshType mesh)
{
    sr.texture = (texturePath != nullptr) ? AEGfxTextureLoad(texturePath) : nullptr;
    sr.width = width;
    sr.height = height;
    sr.meshType = mesh;
}

// ============================================================================
// DrawSpriteRenderer
// ============================================================================
// Applies the transform, sets render mode, and draws the appropriate mesh.
//
// COLOUR RENDERING (no texture):
//   SetColorToMultiply is zeroed out so it doesn't tint the black base.
//   SetColorToAdd provides the actual RGBA colour.
//
// TEXTURE RENDERING:
//   SetColorToMultiply is white (1,1,1,a) to preserve texture colour.
//   colour.a controls transparency.
// ============================================================================
void DrawSpriteRenderer(const SpriteRenderer& sr, Transform& transform)
{
    if (sr.texture != nullptr)
    {
        // Texture mode: draw the image with optional alpha fade
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxTextureSet(sr.texture, sr.uOffset, sr.vOffset);
        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, sr.colour.a);
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    }
    else
    {
        // Colour mode: draw a flat-coloured shape
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.0f);
        AEGfxSetColorToAdd(sr.colour.r, sr.colour.g, sr.colour.b, sr.colour.a);
    }

    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);

    // Build and apply the SRT matrix
    transform.Apply();

    AEGfxVertexList* meshToDraw = nullptr;

    // Select mesh based on type
    if (sr.customMesh != nullptr)
    {
        // If we gave it an animated spritesheet mesh, prioritize that
        meshToDraw = sr.customMesh;
    }
    else
    {
        switch (sr.meshType)
        {
        case MESH_TRIANGLE: meshToDraw = Meshes::pTriangleMesh;   break;
        case MESH_CIRCLE:   meshToDraw = Meshes::pCircleMesh;     break;
        case MESH_SQUARE:   
        default:            meshToDraw = Meshes::pSquareCOriMesh; break;
        }
    }

    AEGfxMeshDraw(meshToDraw, AE_GFX_MDM_TRIANGLES);
}

// ============================================================================
// CreateSpriteSheetMesh
// ============================================================================
// Builds a custom 1x1 square mesh, but scales the UV coordinates down 
// so it only renders a single frame of a larger spritesheet.
// ============================================================================
AEGfxVertexList* CreateSpriteSheetMesh(f32 frameWidth, f32 frameHeight, f32 texWidth, f32 texHeight)
{
    return Meshes::CreateSpriteSheetMesh(frameWidth, frameHeight, texWidth, texHeight);
}

// ============================================================================
// FreeSpriteRenderer
// ============================================================================
// Unloads the texture if one was loaded. Safe to call multiple times.
// ============================================================================
void FreeSpriteRenderer(SpriteRenderer& sr)
{
    if (sr.texture != nullptr)
    {
        AEGfxTextureUnload(sr.texture);
        sr.texture = nullptr;
    }
}