#include "pch.h"
#include "GamePage.h"
#include "StateManager.h"
#include "ExpUI.h"
#include "MiniMap.h"
#include "Player.h"
#include "Enemy.h"
#include <vector>

// ---------------------------------------------------------------------------
// World Settings
// ---------------------------------------------------------------------------
const float WORLD_WIDTH = 2000.0f;
const float WORLD_HEIGHT = 2000.0f;
const float GRID_SIZE = 100.0f;

// ---------------------------------------------------------------------------
// Global/Static Variables
// ---------------------------------------------------------------------------
static Player sPlayer;
static std::vector<Enemy> sEnemy;

const float ENEMY_NO = 10.0f; // Number of Enemies

// Camera variables
static float sCamX = 0.0f;
static float sCamY = 0.0f;
const float CAM_SPEED = 5.0f; // Camera Speed (higher = smoother, lower = sharper)

void Game_Load()
{
    Meshes::CreateTriangleMesh();
    Meshes::CreateSquareLeftOriginMesh();
    Meshes::CreateSquareCenterOriginMesh();
}

void Game_Init()
{
    sEnemy.clear();

    // Initialize Player
    sPlayer.Init();

    // Reset Camera to player
    sCamX = sPlayer.pos.x;
    sCamY = sPlayer.pos.y;

    // Initialize Enemies
    for (int i = 0; i < ENEMY_NO; ++i) {
        Enemy enemy;
        float rX = (rand() % (int)WORLD_WIDTH) - (WORLD_WIDTH / 2.0f);
        float rY = (rand() % (int)WORLD_HEIGHT) - (WORLD_HEIGHT / 2.0f);

        enemy.Init(rX, rY);
        sEnemy.push_back(enemy);
    }
}

void Game_Update()
{
    float dt = (float)AEFrameRateControllerGetFrameTime();

    // Update Player
    sPlayer.Update(dt, WORLD_WIDTH, WORLD_HEIGHT);

    // Smooth Camera Follow
    sCamX += (sPlayer.pos.x - sCamX) * CAM_SPEED * dt;
    sCamY += (sPlayer.pos.y - sCamY) * CAM_SPEED * dt;

    // Game State Controls
    if (AEInputCheckTriggered(AEVK_R)) current = STATE_RESTART;
    if (AEInputCheckTriggered(AEVK_Q)) next = STATE_MENU;

    // Update Enemies
    for (auto& enemy : sEnemy) {
        enemy.Update(dt);
    }
}

void Game_Draw()
{
    // Camera Follow
    AEGfxSetCamPosition(sCamX, sCamY);

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

    // Vertical Lines
    for (float gx = -WORLD_WIDTH / 2; gx <= WORLD_WIDTH / 2; gx += GRID_SIZE) {
        AEMtx33Scale(&s, 2.0f, WORLD_HEIGHT);
        AEMtx33Trans(&t, gx, 0.0f);
        AEMtx33Concat(&transform, &t, &s);
        AEGfxSetTransform(transform.m);
        AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);
    }
    // Horizontal Lines
    for (float gy = -WORLD_HEIGHT / 2; gy <= WORLD_HEIGHT / 2; gy += GRID_SIZE) {
        AEMtx33Scale(&s, WORLD_WIDTH, 2.0f);
        AEMtx33Trans(&t, 0.0f, gy);
        AEMtx33Concat(&transform, &t, &s);
        AEGfxSetTransform(transform.m);
        AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);
    }

    // Draw Objects
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    sPlayer.Draw();

    for (auto& enemy : sEnemy) {
        enemy.Draw();
    }

    // -----------------------------------------------------------------------
    // Draw UI (HUD)
    // -----------------------------------------------------------------------

    // XP Bar
    // DrawExpBar(PlayerExp, -700.0f + camX, 400.0f + camY, 200.0f, 30.0f);

    // Minimap
    DrawMinimap(sPlayer, sEnemy, sCamX, sCamY);
}

void Game_Free()
{
    sEnemy.clear();
}

void Game_Unload()
{
    Meshes::FreeMeshes();
}