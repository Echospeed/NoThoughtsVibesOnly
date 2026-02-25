// ============================================================================
// MiniMap.cpp - Minimap Overlay Renderer
// ============================================================================
// Draws a small overhead map in the top-right corner of the screen showing:
//   - A dark semi-transparent background
//   - The player as a red dot
//   - All visible, active NPCs as small yellow dots
//
// The minimap is drawn in world space (camera must be set before calling).
// World coordinates are scaled down by MINI_SCALE to fit inside MINI_SIZE.
//
// USAGE (from Game_Draw, before camera is reset to 0,0):
// ----------------------------------------------------------------------------
//   DrawMinimap(gamePageObj, sCamX, sCamY);
// ============================================================================

#include "pch.hpp"
#include "MiniMap.hpp"
#include "NPC.hpp"

// ============================================================================
// Minimap Constants
// ============================================================================
static const f32 MINI_WORLD_REF_WIDTH = 2000.0f; // World width the map represents
static const f32 MINI_SIZE = 200.0f;   // Minimap display size (px)
static const f32 MINI_X = 650.0f;   // Screen-space X offset from center
static const f32 MINI_Y = 300.0f;   // Screen-space Y offset from center
static const f32 MINI_SCALE = MINI_SIZE / MINI_WORLD_REF_WIDTH;

// ============================================================================
// DrawMinimap
// ============================================================================
// objs : All game objects to render (player + enemies + bullets)
// camX : Current camera world X (used to position map in screen space)
// camY : Current camera world Y
// ============================================================================
void DrawMinimap(const std::vector<GameObject*>& objs, f32 camX, f32 camY)
{
    // The minimap background is positioned relative to the camera so it
    // stays fixed in screen space while the world camera moves
    const f32 mapWorldX = MINI_X + camX;
    const f32 mapWorldY = MINI_Y + camY;

    AEMtx33 scale, trans, rot, transform;

    // --- Minimap background (semi-transparent black) ---
    AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.5f);
    CreateSquare(Meshes::pSquareCOriMesh, &transform, &scale, &rot, &trans,
        mapWorldX, mapWorldY,
        MINI_SIZE, MINI_SIZE, 0.0f,
        0.0f, 0.0f, 0.0f, 0.5f);

    // --- Draw each object onto the minimap ---
    for (size_t i = 0; i < objs.size(); ++i)
    {
        const GameObject* entity = objs[i];
        if (!entity || !entity->isActive) continue;

        // Player is assumed to be the first object in the list (index 0)
        if (i == 0)
        {
            const f32 px = mapWorldX + entity->transform.position.x * MINI_SCALE;
            const f32 py = mapWorldY + entity->transform.position.y * MINI_SCALE;

            // Red dot for the player
            CreateSquare(Meshes::pSquareCOriMesh, &transform, &scale, &rot, &trans,
                px, py, 8.0f, 8.0f, 0.0f,
                1.0f, 0.0f, 0.0f, 1.0f);
            continue;
        }

        // Only show NPCs that the player can see
        if (entity->ObjectType == NP)
        {
            const NPC* npc = dynamic_cast<const NPC*>(entity);
            if (!npc || !npc->isVisibleToPlayer) continue;
        }

        // Small yellow dot for all other visible entities
        const f32 ex = mapWorldX + entity->transform.position.x * MINI_SCALE;
        const f32 ey = mapWorldY + entity->transform.position.y * MINI_SCALE;

        CreateSquare(Meshes::pCircleMesh, &transform, &scale, &rot, &trans,
            ex, ey, 2.0f, 2.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f);
    }
}