#include "pch.h"
#include "MiniMap.h"

// ---------------------------------------------------------------------------
// Minimap Constants
// ---------------------------------------------------------------------------.
const float MINI_WORLD_REF_WIDTH = 2000.0f;

const float MINI_SIZE = 200.0f;
const float MINI_X = 650.0f; // Screen Offset X
const float MINI_Y = 300.0f; // Screen Offset Y
const float MINI_SCALE = MINI_SIZE / MINI_WORLD_REF_WIDTH;

void DrawMinimap(const Player& player, const std::vector<Enemy>& enemies, float camX, float camY) 
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

    // Draw Player Dot
    // Logic: Map Origin + (Player Position * Scale)
    float playerMiniX = mapWorldX + (player.pos.x * MINI_SCALE);
    float playerMiniY = mapWorldY + (player.pos.y * MINI_SCALE);

    CreateSquare(Meshes::pSquareCOriMesh, &transform, &scale, &rot, &trans,
        playerMiniX, playerMiniY, 8.0f, 8.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 1.0f);

    // Draw Enemy Dots
    for (const auto& enemy : enemies) {
        float enemyMiniX = mapWorldX + (enemy.pos.x * MINI_SCALE);
        float enemyMiniY = mapWorldY + (enemy.pos.y * MINI_SCALE);

        CreateSquare(Meshes::pSquareCOriMesh, &transform, &scale, &rot, &trans,
            enemyMiniX, enemyMiniY, 6.0f, 6.0f, 0.0f,
            1.0f, 1.0f, 0.0f, 1.0f);
    }
}