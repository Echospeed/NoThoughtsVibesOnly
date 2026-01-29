#include "pch.h"
#include "MiniMap.h"

// ---------------------------------------------------------------------------
// Minimap Constants
// ---------------------------------------------------------------------------.
const float MINI_WORLD_REF_WIDTH = 2000.0f;

const float MINI_SIZE = 200.0f;
const float MINI_X = 650.0f;    // Screen Offset X
const float MINI_Y = 300.0f;    // Screen Offset Y
const float MINI_SCALE = MINI_SIZE / MINI_WORLD_REF_WIDTH;

void DrawMinimap(const std::vector<GameObject*>& objs, float camX, float camY)
{

    // Calculate World Coordinates for the Minimap Background
    float mapWorldX = MINI_X + camX;
    float mapWorldY = MINI_Y + camY;

    AEMtx33 scale, trans, rot, transform;

    // Draw Minimap Background
    AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.5f);

    CreateSquare(Meshes::pSquareCOriMesh, &transform, &scale, &rot, &trans,
        mapWorldX, mapWorldY, MINI_SIZE, MINI_SIZE, 0.0f,
        0.0f, 0.0f, 0.0f, 0.5f);

    // Draw Enemy Dots
    for (const auto& entity : objs) 
    {
        if (objs.at(0) == entity)
        {
            // Logic: Map Origin + (Player Position * Scale)
            float playerMiniX = mapWorldX + (entity->transform.position.x * MINI_SCALE);
            float playerMiniY = mapWorldY + (entity->transform.position.y * MINI_SCALE);
            CreateSquare(Meshes::pSquareCOriMesh, &transform, &scale, &rot, &trans, playerMiniX, playerMiniY, 8.0f, 8.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f);
            continue;
        }

        
        float entityX = mapWorldX + (entity->transform.position.x * MINI_SCALE);
        float entityY = mapWorldY + (entity->transform.position.y * MINI_SCALE);

        CreateSquare(Meshes::pSquareCOriMesh, &transform, &scale, &rot, &trans,
            entityX, entityY, 6.0f, 6.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f);
    }
}