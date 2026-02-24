// ============================================================================
// SpriteRenderer.cpp - IMPROVED VERSION
// ============================================================================
// Handles rendering of sprites and colored shapes in the game.
// Supports textures, colors, and different mesh types (square, triangle, circle)
// 
// CRITICAL BUG FIX: Fixed inverted texture/color rendering logic
// ============================================================================

#include "pch.hpp"
#include "AEEngine.h"
#include "Util.hpp"
#include "SpriteRenderer.hpp"

// Global mesh pointer used for rendering
// This is set based on the meshType of the sprite being drawn
AEGfxVertexList* list{ nullptr };

// ============================================================================
// InitSpriteRenderer
// ============================================================================
// Initializes a sprite renderer with texture, dimensions, and mesh type
// 
// Parameters:
//   spriteRenderer - The sprite renderer to initialize
//   texturePath    - Path to texture file (can be nullptr for colored shapes)
//   width          - Width of the sprite
//   height         - Height of the sprite  
//   mesh           - Type of mesh to use (MESH_SQUARE, MESH_TRIANGLE, MESH_CIRCLE)
// ============================================================================
void InitSpriteRenderer(SpriteRenderer& spriteRenderer, const char* texturePath, f32 width, f32 height, MeshType mesh)
{
    // Load texture if path is provided, otherwise set to nullptr for colored rendering
    spriteRenderer.texture = (texturePath != nullptr) ? AEGfxTextureLoad(texturePath) : nullptr;
    spriteRenderer.width = width;
    spriteRenderer.height = height;
    spriteRenderer.meshType = mesh;
}

// ============================================================================
// DrawSpriteRenderer - BUG FIX APPLIED
// ============================================================================
// Renders the sprite using its transform, texture, color, and mesh type
// 
// CRITICAL FIX: The original code had inverted logic:
//   OLD: texture EXISTS → set COLOR mode (WRONG)
//   OLD: texture is NULL → set TEXTURE mode (WRONG)
// NEW: texture EXISTS → set TEXTURE mode (CORRECT)
// NEW: texture is NULL → set COLOR mode (CORRECT)
// ============================================================================
void DrawSpriteRenderer(const SpriteRenderer& spriteRenderer, Transform& transform)
{
    // ========================================================================
    // CRITICAL FIX: Corrected render mode logic
    // ========================================================================
    if (spriteRenderer.texture != nullptr)
    {
        // We have a texture - use TEXTURE rendering mode
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxTextureSet(spriteRenderer.texture, 0, 0);
        
        // For textured sprites, multiply keeps the texture as-is
        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, spriteRenderer.colour.a);
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    }
    else
    {
        // No texture - use pure COLOR rendering mode
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        
        // Color mode uses ColorToAdd for the actual color
        AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.0f);
        AEGfxSetColorToAdd(spriteRenderer.colour.r, spriteRenderer.colour.g, 
                           spriteRenderer.colour.b, spriteRenderer.colour.a);
    }
    
    // Set transparency and blend mode for proper alpha rendering
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);
    
    // Apply the transform (position, rotation, scale) to the rendering matrix
    TransformMovement(transform);
    
    // Select the appropriate mesh based on the sprite's mesh type
    switch (spriteRenderer.meshType)
    {
        case MESH_SQUARE:
            list = Meshes::pSquareCOriMesh;
            break;
        case MESH_TRIANGLE:
            list = Meshes::pTriangleMesh;
            break;
        case MESH_CIRCLE:
            list = Meshes::pCircleMesh;
            break;
        default:
            list = Meshes::pSquareCOriMesh; // Default to square if unknown
            break;
    }
    
    // Draw the mesh with triangles
    AEGfxMeshDraw(list, AE_GFX_MDM_TRIANGLES);
}

// ============================================================================
// FreeSpriteRenderer
// ============================================================================
// Cleans up sprite renderer resources
// Unloads texture if one was loaded
// ============================================================================
void FreeSpriteRenderer(SpriteRenderer& spriteRenderer)
{
    // Only unload texture if one exists
    if(spriteRenderer.texture != nullptr)
    {
        AEGfxTextureUnload(spriteRenderer.texture);
        spriteRenderer.texture = nullptr;
    }
    
    // Clear the mesh list pointer
    list = nullptr;
}
