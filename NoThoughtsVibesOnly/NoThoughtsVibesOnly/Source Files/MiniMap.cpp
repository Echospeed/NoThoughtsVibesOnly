#include "pch.h"
#include "MiniMap.h"
#include "NPC.h"

// ---------------------------------------------------------------------------
// Minimap Constants
// ---------------------------------------------------------------------------
const f32 MINI_WORLD_REF_WIDTH = 2000.0f;
const f32 MINI_SIZE = 200.0f;
const f32 MINI_X = 650.0f;    // Screen Offset X
const f32 MINI_Y = 300.0f;    // Screen Offset Y
const f32 MINI_SCALE = MINI_SIZE / MINI_WORLD_REF_WIDTH;

void DrawMinimap(const std::vector<GameObject*>& objs, f32 camX, f32 camY)
{
    f32 mapWorldX = MINI_X + camX;
    f32 mapWorldY = MINI_Y + camY;

    AEMtx33 scale, trans, rot, transform;

    // Draw Minimap Background
    AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.5f);
    CreateSquare(Meshes::pSquareCOriMesh, &transform, &scale, &rot, &trans,
        mapWorldX, mapWorldY, MINI_SIZE, MINI_SIZE, 0.0f,
        0.0f, 0.0f, 0.0f, 0.5f);

    // Draw Objects
    for (size_t i = 0; i < objs.size(); ++i)
    {
        GameObject* entity = objs[i];
        if (!entity || !entity->isActive) continue;

        // Player dot (assume player is first object)
        if (i == 0)
        {
            f32 playerMiniX = mapWorldX + entity->transform.position.x * MINI_SCALE;
            f32 playerMiniY = mapWorldY + entity->transform.position.y * MINI_SCALE;
            CreateSquare(Meshes::pSquareCOriMesh, &transform, &scale, &rot, &trans,
                playerMiniX, playerMiniY, 8.0f, 8.0f, 0.0f,
                1.0f, 0.0f, 0.0f, 1.0f);
            continue;
        }

        // Skip invisible NPCs
        if (entity->ObjectType == NP)
        {
            NPC* npc = dynamic_cast<NPC*>(entity);
            if (!npc || !npc->isVisibleToPlayer) continue;
        }

        // Draw other objects (enemies, bullets)
        f32 entityMiniX = mapWorldX + entity->transform.position.x * MINI_SCALE;
        f32 entityMiniY = mapWorldY + entity->transform.position.y * MINI_SCALE;

        CreateSquare(Meshes::pSquareCOriMesh, &transform, &scale, &rot, &trans,
            entityMiniX, entityMiniY, 6.0f, 6.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f);
    }

}
