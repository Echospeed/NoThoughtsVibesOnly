#include "pch.h"
#include "GamePage.h"
#include "StateManager.h"
#include "MiniMap.h"
#include "Player.h"
#include "Enemy.h"
#include "AIEnemy.h"
#include <vector>
#include <iostream>


// ---------------------------------------------------------------------------
// World Settings
// ---------------------------------------------------------------------------
const float WORLD_WIDTH = 2000.0f;
const float WORLD_HEIGHT = 2000.0f;
const float GRID_SIZE = 100.0f;

// ---------------------------------------------------------------------------
// Global/Static Object Vectors $DO NOT MOVE, MUST INITIALISE BEFORE USE$
// ---------------------------------------------------------------------------
std::vector<GameObject*> objects;

// ---------------------------------------------------------------------------
// Global/Static GameObjects and Variables
// ---------------------------------------------------------------------------
GameObject* pPlayer{nullptr};



// Experience PlayerExp;

static float sCamX = 0.0f;
static float sCamY = 0.0f;
const float CAM_SPEED = 5.0f; // Higher = tighter, Lower = smoother/looser

void Game_Load()
{
    Meshes::CreateTriangleMesh();
    Meshes::CreateSquareLeftOriginMesh();
    Meshes::CreateSquareCenterOriginMesh();
	Meshes::CreateCircleMesh();
}

void Game_Init()
{
    // Create the player first
    Player* player = new Player();
    pPlayer = player; // optional for camera follow

    sCamX = player->transform.position.x;
    sCamY = player->transform.position.y;

    // Spawn regular enemies
    for (int i = 0; i < 3; ++i)
        new Enemy(player); // optional, regular enemies

    // Spawn AI enemies with player reference
    for (int i = 0; i < 5; ++i)
        new AIEnemy(player); // pass player pointer

    // Call Start() for all objects
    for (auto& obj : objects)
        obj->Start();

    std::cout << "Game_Init running\n";
}


void Game_Update()
{
    float dt = (float)AEFrameRateControllerGetFrameTime();

	//testObject.Update(dt);

    // Boundary Logic
    float halfWorldWidth = WORLD_WIDTH / 2.0f;
    float halfWorldHeight = WORLD_HEIGHT / 2.0f;
    float halfPlayerSizex = (pPlayer->transform.scale.x / 2.0f);
    float halfPlayerSizey = (pPlayer->transform.scale.y / 2.0f);

    if (pPlayer->transform.position.x > halfWorldWidth - halfPlayerSizex) pPlayer->transform.position.x = halfWorldWidth - halfPlayerSizex;
    if (pPlayer->transform.position.x < -halfWorldWidth + halfPlayerSizex) pPlayer->transform.position.x = -halfWorldWidth + halfPlayerSizex;
    if (pPlayer->transform.position.y > halfWorldHeight - halfPlayerSizey) pPlayer->transform.position.y = halfWorldHeight - halfPlayerSizey;
    if (pPlayer->transform.position.y < -halfWorldHeight + halfPlayerSizey) pPlayer->transform.position.y = -halfWorldHeight + halfPlayerSizey;

    // Smooth Camera Follow (Lerp)
    // This calculates the difference between player and camera, and moves a fraction of that distance
    sCamX += (pPlayer->transform.position.x - sCamX) * CAM_SPEED * dt;
    sCamY += (pPlayer->transform.position.y - sCamY) * CAM_SPEED * dt;

    // -----------------------------------------------------------------------
    // Game State Controls
    // -----------------------------------------------------------------------
    for (auto& obj : objects) {
        obj->Update(dt);
    }

    // Restart Level
    if (AEInputCheckTriggered(AEVK_R))
    {
        StateManagerChangeState(STATE_RESTART);
    }
    // Return to Main Menu
    if (AEInputCheckTriggered(AEVK_Q))
    {
        StateManagerChangeState(STATE_MENU);
    }

}

void Game_Draw()
{
    // -----------------------------------------------------------------------
    // Camera Follow
    // -----------------------------------------------------------------------
    AEGfxSetCamPosition(sCamX, sCamY);

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

    // -----------------------------------------------------------------------
    // Draw UI (HUD)
    // -----------------------------------------------------------------------

    // XP Bar
    // DrawExpBar(PlayerExp, -700.0f + camX, 400.0f + camY, 200.0f, 30.0f);


    // Minimap
    DrawMinimap(objects, sCamX, sCamY);
    
    for (auto& obj : objects) {
        obj->Draw();
    }
}

void Game_Free()
{
    for (auto& obj : objects)
    {
        delete obj;
        obj = nullptr;
    }

    objects.clear();

    pPlayer = nullptr;
	//FreeSpriteRenderer(testObject.spriteRenderer);
}

void Game_Unload()
{
    Meshes::FreeMeshes();
}