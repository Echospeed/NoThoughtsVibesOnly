#include "pch.h"
#include "GamePage.h"
#include "StateManager.h"
#include <vector>
#include "ExpUI.h"
// #include "MiniMap.h"

// ---------------------------------------------------------------------------
// World Settings
// ---------------------------------------------------------------------------
const float WORLD_WIDTH = 2000.0f;
const float WORLD_HEIGHT = 2000.0f;
const float GRID_SIZE = 100.0f;

// ---------------------------------------------------------------------------
// Global/Static Variables
// ---------------------------------------------------------------------------
static std::vector<GameObject> sEnemyList;
static GameObject sPlayer;
static float playerSpeed = 300.0f;

Experience PlayerExp;

void Game_Load()
{
    Meshes::CreateTriangleMesh();
    Meshes::CreateSquareLeftOriginMesh();
    Meshes::CreateSquareCenterOriginMesh();
}

void Game_Init()
{
    sEnemyList.clear();

    // Initialize Player
    sPlayer.pos = { 0.0f, 0.0f };
    sPlayer.scale = 50.0f;
    sPlayer.color = 0xFFFF0000; // Red
    sPlayer.pMesh = Meshes::pSquareCOriMesh;
    sPlayer.velocity = { 0.0f, 0.0f };

    // Initialize Enemies
    for (int i = 0; i < 5; ++i) {
        GameObject enemy;
        float rX = (rand() % (int)WORLD_WIDTH) - (WORLD_WIDTH / 2.0f);
        float rY = (rand() % (int)WORLD_HEIGHT) - (WORLD_HEIGHT / 2.0f);

        enemy.pos = { rX, rY };
        enemy.pMesh = Meshes::pTriangleMesh;
        enemy.color = 0xFFFFFF00; // Yellow
        enemy.scale = 1.0f;
        sEnemyList.push_back(enemy);
    }
}

void Game_Update()
{
    float dt = (float)AEFrameRateControllerGetFrameTime();

    // Reset Velocity
    sPlayer.velocity = { 0.0f, 0.0f };

    // Player Input
    if (AEInputCheckCurr(AEVK_W)) sPlayer.velocity.y += playerSpeed;
    if (AEInputCheckCurr(AEVK_S)) sPlayer.velocity.y -= playerSpeed;
    if (AEInputCheckCurr(AEVK_A)) sPlayer.velocity.x -= playerSpeed;
    if (AEInputCheckCurr(AEVK_D)) sPlayer.velocity.x += playerSpeed;

    // Apply Movement
    sPlayer.Update(dt);

    // Boundary Logic
    float halfWorldWidth = WORLD_WIDTH / 2.0f;
    float halfWorldHeight = WORLD_HEIGHT / 2.0f;
    float halfPlayerSize = sPlayer.scale / 2.0f;

    if (sPlayer.pos.x > halfWorldWidth - halfPlayerSize) sPlayer.pos.x = halfWorldWidth - halfPlayerSize;
    if (sPlayer.pos.x < -halfWorldWidth + halfPlayerSize) sPlayer.pos.x = -halfWorldWidth + halfPlayerSize;
    if (sPlayer.pos.y > halfWorldHeight - halfPlayerSize) sPlayer.pos.y = halfWorldHeight - halfPlayerSize;
    if (sPlayer.pos.y < -halfWorldHeight + halfPlayerSize) sPlayer.pos.y = -halfWorldHeight + halfPlayerSize;

    // -----------------------------------------------------------------------
    // Game State Controls
    // -----------------------------------------------------------------------

    // Restart Level
    if (AEInputCheckTriggered(AEVK_R)) current = STATE_RESTART;

    // Return to Main Menu
    if (AEInputCheckTriggered(AEVK_Q)) next = STATE_MENU;

    for (auto& enemy : sEnemyList) {
        enemy.Update(dt);
    }
}

void Game_Draw()
{
    // -----------------------------------------------------------------------
    // Camera Follow
    // -----------------------------------------------------------------------
    AEGfxSetCamPosition(sPlayer.pos.x, sPlayer.pos.y);

    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

    // -----------------------------------------------------------------------
    // Draw World
    // -----------------------------------------------------------------------
    AEMtx33 transform, s, t;

    // Draw Border (Red Box)
    AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 1.0f);
    AEMtx33Scale(&s, WORLD_WIDTH + 20.0f, WORLD_HEIGHT + 20.0f);
    AEMtx33Trans(&t, 0.0f, 0.0f);
    AEMtx33Concat(&transform, &t, &s);
    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

    // Draw Floor (Grey Box)
    AEGfxSetColorToMultiply(0.3f, 0.3f, 0.3f, 1.0f);
    AEMtx33Scale(&s, WORLD_WIDTH, WORLD_HEIGHT);
    AEMtx33Concat(&transform, &t, &s);
    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

    // Draw Background (with Grids)
    AEGfxSetColorToMultiply(0.5f, 0.5f, 0.5f, 1.0f);

    

    // Draw Objects
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    sPlayer.Draw();

    for (auto& enemy : sEnemyList) {
        enemy.Draw();
    }

    // -----------------------------------------------------------------------
    // Draw UI (HUD)
    // -----------------------------------------------------------------------

    // XP Bar
    // DrawExpBar(PlayerExp, -700.0f + camX, 400.0f + camY, 200.0f, 30.0f);

    // Minimap
    // DrawMinimap(sPlayer, sEnemyList);
}

void Game_Free()
{
    sEnemyList.clear();
}

void Game_Unload()
{
    Meshes::FreeMeshes();
}