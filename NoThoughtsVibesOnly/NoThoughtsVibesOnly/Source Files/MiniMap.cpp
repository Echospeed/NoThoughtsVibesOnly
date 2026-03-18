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
// DrawMinimap
// ============================================================================
// objs : All game objects to render (player + enemies + bullets)
// camX : Current camera world X (used to position map in screen space)
// camY : Current camera world Y
// ============================================================================
void DrawMinimap(const std::vector<GameObject*>& objs, f32 camX, f32 camY)
{
    const auto& mm = GameConfig::Gameplay().minimap;
    const f32 MINI_SIZE = mm.size;
    const f32 MINI_X = mm.screenOffsetX;
    const f32 MINI_Y = mm.screenOffsetY;
    const f32 MINI_SCALE = MINI_SIZE / mm.worldRefWidth;
    const f32 mapWorldX = MINI_X + camX;
    const f32 mapWorldY = MINI_Y + camY;

    AEMtx33 scale, trans, rot, transform;

    // Minimap background (semi-transparent white)
    // AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 0.5f);
    CreateSquare(Meshes::pSquareCOriMesh, &transform, &scale, &rot, &trans,
        mapWorldX, mapWorldY,
        MINI_SIZE, MINI_SIZE, 0.0f,
        1.0f, 1.0f, 1.0f, 0.5f);

    // Draw each object onto the minimap
    for (size_t i = 0; i < objs.size(); ++i)
    {
        const GameObject* entity = objs[i];
        if (!entity || !entity->isActive) continue;

        if (i == 0)
        {
            const f32 px = mapWorldX + entity->transform.position.x * MINI_SCALE;
            const f32 py = mapWorldY + entity->transform.position.y * MINI_SCALE;

            // Dynamically pull the player's current color
            // Dynamically pull the player's current color
            CreateSquare(Meshes::pSquareCOriMesh, &transform, &scale, &rot, &trans,
                px, py, mm.playerDotSize, mm.playerDotSize, 0.0f,
                entity->spriteRenderer.colour.r,
                entity->spriteRenderer.colour.g,
                entity->spriteRenderer.colour.b,
                1.0f);
        }

        // Filter out everything that isn't an NPC
        if (entity->ObjectType != NP) continue;

        // Only show NPCs that the player can see
        const NPC* npc = dynamic_cast<const NPC*>(entity);
        if (!npc || !npc->isVisibleToPlayer) continue;

        // Small dot for visible NPCs, matching their specific type color
        const f32 ex = mapWorldX + entity->transform.position.x * MINI_SCALE;
        const f32 ey = mapWorldY + entity->transform.position.y * MINI_SCALE;

        // Small dot for visible NPCs, matching their specific type color
        CreateSquare(Meshes::pCircleMesh, &transform, &scale, &rot, &trans,
            ex, ey, mm.npcDotSize, mm.npcDotSize, 0.0f,
            npc->baseColour.r,
            npc->baseColour.g,
            npc->baseColour.b,
            1.0f);
    }
}