#include "pch.hpp"
#include "GamePage.hpp"
#include "StateManager.hpp"
#include "ExpUI.hpp"
#include "MiniMap.hpp"
#include "Player.hpp"
#include "NPC.hpp"
#include "GameObjectType.hpp"
#include "pch.h"
#include "GamePage.h"
#include "StateManager.h"
#include "GameUI.h"
#include "MiniMap.h"
#include "Player.h"
#include "NPC.h"
#include "GameObjectType.h"
#include <vector>
#include <iostream>
#include "Bullet.hpp"
#include "WaveSystem.hpp"
#include "PowerUpSystem.hpp"
#include "GameUI.hpp"           // ← NEW: replaces all the UI function declarations

// ---------------------------------------------------------------------------
// World Settings
// ---------------------------------------------------------------------------
const f32 WORLD_WIDTH = 2000.0f;
const f32 WORLD_HEIGHT = 2000.0f;
const f32 GRID_SIZE = 100.0f;
int availableBullets = 0;

// ---------------------------------------------------------------------------
// Global/Static Object Vectors $DO NOT MOVE, MUST INITIALISE BEFORE USE$
// ---------------------------------------------------------------------------
std::vector<GameObject*> gamePageObj;

// ---------------------------------------------------------------------------
// Global/Static GamePage and Variables
// ---------------------------------------------------------------------------
GameObject* pPlayer{ nullptr };
static bool isPaused = false;

// ---------------------------------------------------------------------------
// Systems
// ---------------------------------------------------------------------------
PowerUpSystem powerUpSystem;
WaveSystem    waveSystem;
GameUI        gameUI;           // ← NEW: single UI object

// ---------------------------------------------------------------------------
// Font / Ammo text
// ---------------------------------------------------------------------------
TextRenderer ammoText;
s8           gameFont = 0;
char         ammoBuffer[500];
//TextRenderer ammoText;
//s8 gameFont = 0; // Handle for the font
//char ammoBuffer[500]; // Buffer to hold the text string "Ammo: 50"

// ---------------------------------------------------------------------------
// Camera
// ---------------------------------------------------------------------------
static f32    sCamX = 0.0f;
static f32    sCamY = 0.0f;
const  f32    CAM_SPEED = 5.0f;

// ============================================================================
// Game_Load
// ============================================================================
// Game UI
GameUI UIelements{};


void Game_Load()
{
    Meshes::CreateTriangleMesh();
    Meshes::CreateSquareLeftOriginMesh();
    Meshes::CreateSquareCenterOriginMesh();
    Meshes::CreateCircleMesh();
    gameFont = AEGfxCreateFont("Assets/buggy-font.ttf", 30);
    LoadTextRenderer(ammoText, gameFont);
    // Load the font
    //gameFont = AEGfxCreateFont("Assets/buggy-font.ttf", 30);
    //LoadTextRenderer(ammoText, gameFont);

    // UI Elements
    UIelements.Load();
}

// ============================================================================
// Game_Init
// ============================================================================
void Game_Init()
{
    // Player
    // UI Elements
    UIelements.Init();

    // Initialize Player
    pPlayer = new Player();
    sCamX = pPlayer->transform.position.x;
    sCamY = pPlayer->transform.position.y;

    InitTextRenderer(ammoText, "Ammo: 100%", { 1.0f,1.0f }, 1.0f, 1.0f, 1.0f);
    // Initialize Ammo UI (Top Left corner usually)
    // Position x=-700, y=400 puts it in top-left area relative to camera center if UI follows camera
    // But since we draw UI *after* resetting camera, we use screen coordinates logic.
    //InitTextRenderer(ammoText, "Ammo: 100%", 1.0f, 1.0f, 1.0f, 1.0f); // White text

    // Systems
    waveSystem.Init(dynamic_cast<Player*>(pPlayer));
    powerUpSystem.Init();

    Player* player = dynamic_cast<Player*>(pPlayer);
    if (player) player->powerUpSystem = &powerUpSystem;

    // ⭐ Init GameUI - one call, done
    gameUI.Init(gameFont, &waveSystem, &powerUpSystem);

    // Bullet pool - Player
    for (int i = 0; i < 500; ++i)
    {
        Bullet* bullet = new Bullet();
        bullet->startPos = pPlayer;
        bullet->isActive = false;
        bullet->owner = BulletOwner::PLAYER;
        bullet->spriteRenderer.colour = { 1.0f, 1.0f, 0.0f, 0.0f };
    }

    // Bullet pool - Enemy
    for (int i = 0; i < 100; ++i)
    {
        Bullet* bullet = new Bullet();
        bullet->owner = BulletOwner::ENEMY;
        bullet->isActive = false;
        bullet->spriteRenderer.colour = { 1.0f, 0.0f, 0.0f, 0.0f };
    }

    for (auto& obj : gamePageObj)
        obj->Start();
}

// ============================================================================
// Game_Update
// ============================================================================
void Game_Update()
{
    // Pause toggle
    if (AEInputCheckTriggered(AEVK_ESCAPE))
    {
        isPaused = !isPaused;
        return;
    }

    if (isPaused)
    {
        if (AEInputCheckTriggered(AEVK_R)) { isPaused = false; StateManagerChangeState(STATE_RESTART); return; }
        if (AEInputCheckTriggered(AEVK_Q)) { isPaused = false; StateManagerChangeState(STATE_MENU);    return; }
        return;
    }

    f32 dt = (f32)AEFrameRateControllerGetFrameTime();

    // Power-up selection (blocks game update while choosing)
    if (powerUpSystem.IsWaitingForUpgrade())
    {
        PowerUp* choices = powerUpSystem.GetPowerUpChoices();
        Player* player = dynamic_cast<Player*>(pPlayer);
        if (AEInputCheckTriggered(AEVK_1)) powerUpSystem.ApplyPowerUp(choices[0].type, player);
        else if (AEInputCheckTriggered(AEVK_2)) powerUpSystem.ApplyPowerUp(choices[1].type, player);
        else if (AEInputCheckTriggered(AEVK_3)) powerUpSystem.ApplyPowerUp(choices[2].type, player);
        return;
    }
    // UI Elements
    UIelements.Update(dt);
    UIelements.UpdatePositionWithCamera(sCamX, sCamY);
    UIelements.UpdateHealth(dynamic_cast<Player*>(pPlayer)->health, dynamic_cast<Player*>(pPlayer)->maxHealth);
	//testObject.Update(dt);

    // Wave system
    waveSystem.Update(dt);
    if (AEInputCheckTriggered(AEVK_C) && waveSystem.GetCurrentWave() == 0)
        waveSystem.StartNextWave();

    // Boundary clamp
    f32 halfW = WORLD_WIDTH / 2.0f;
    f32 halfH = WORLD_HEIGHT / 2.0f;
    f32 halfPx = pPlayer->transform.scale.x / 2.0f;
    f32 halfPy = pPlayer->transform.scale.y / 2.0f;
    if (pPlayer->transform.position.x > halfW - halfPx) pPlayer->transform.position.x = halfW - halfPx;
    if (pPlayer->transform.position.x < -halfW + halfPx) pPlayer->transform.position.x = -halfW + halfPx;
    if (pPlayer->transform.position.y > halfH - halfPy) pPlayer->transform.position.y = halfH - halfPy;
    if (pPlayer->transform.position.y < -halfH + halfPy) pPlayer->transform.position.y = -halfH + halfPy;

    // Camera lerp
    sCamX += (pPlayer->transform.position.x - sCamX) * CAM_SPEED * dt;
    sCamY += (pPlayer->transform.position.y - sCamY) * CAM_SPEED * dt;

    // Count ammo
    availableBullets = 0;
    for (auto& obj : gamePageObj)
    {
        if (obj->ObjectType == ObjectType::SHOT)
        {
            Bullet* b = (Bullet*)obj;
            if (b->owner == BulletOwner::PLAYER)
            {
                if (!b->isActive) availableBullets++;
            }
        }
    }

    // Player death
    if (dynamic_cast<Player*>(pPlayer)->health <= 0.0f)
    {
        StateManagerChangeState(STATE_FINISH);
        return;
    }

    // Win condition
    if (waveSystem.GetCurrentWave() > 0 && !waveSystem.IsInBreak())
    {
        bool anyEnemiesLeft = false;
        for (GameObject* obj : gamePageObj)
            if (obj && obj->isActive && obj->ObjectType == NP)
            {
                anyEnemiesLeft = true; break;
            }

        if (!anyEnemiesLeft)
        {
            StateManagerChangeState(STATE_WIN);
            return;
        }
    }

    // Update all gamePageObj
    for (auto& obj : gamePageObj)
    {
        if (!obj) continue;
        if (obj->ObjectType == NP)
        {
            NPC* npc = dynamic_cast<NPC*>(obj);
            if (!npc || !obj->isActive) continue;
            npc->Update(dt);
        }
        else
        {
            obj->Update(dt);
        }
    }

    // Update ammo text
    //sprintf_s(ammoBuffer, "Ammo: %d / %d", availableBullets, 500);
	ammoText.SetText("Ammo: ", availableBullets, " / 500");
    //ammoText.text = ammoBuffer;
    // Format the text string
    AEGfxSetCamPosition(0.0f, 0.0f);
    //sprintf_s(ammoBuffer, "Ammo: %d / %d", availableBullets, 500);
    //ammoText.text = ammoBuffer;


    //check if enemy count is 0 to finish level or press space key
    if (AEInputCheckTriggered(AEVK_SPACE)) // space key
    {
        StateManagerChangeState(STATE_FINISH);
    }
	//win condition: if all NPCs are inactive
    if (AEInputCheckTriggered(AEVK_RETURN)) // space key
    {
        StateManagerChangeState(STATE_WIN);
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

    // Hotkeys
    if (AEInputCheckTriggered(AEVK_SPACE))  StateManagerChangeState(STATE_FINISH);
    if (AEInputCheckTriggered(AEVK_RETURN)) StateManagerChangeState(STATE_WIN);
    if (AEInputCheckTriggered(AEVK_R))      StateManagerChangeState(STATE_RESTART);
    if (AEInputCheckTriggered(AEVK_Q))      StateManagerChangeState(STATE_MENU);
}

// ============================================================================
// Game_Draw
// ============================================================================
void Game_Draw()
{
    // -----------------------------------------------------------------------
    // WORLD SPACE
    // -----------------------------------------------------------------------
    AEGfxSetCamPosition(sCamX, sCamY);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

    AEMtx33 transform, s, t;

    // Border
    AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 1.0f);
    AEMtx33Scale(&s, WORLD_WIDTH + 20.0f, WORLD_HEIGHT + 20.0f);
    AEMtx33Trans(&t, 0.0f, 0.0f);
    AEMtx33Concat(&transform, &t, &s);
    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

    // Floor
    AEGfxSetColorToMultiply(0.3f, 0.3f, 0.3f, 1.0f);
    AEMtx33Scale(&s, WORLD_WIDTH, WORLD_HEIGHT);
    AEMtx33Concat(&transform, &t, &s);
    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

    // Grid
    AEGfxSetColorToMultiply(0.5f, 0.5f, 0.5f, 1.0f);
    for (f32 gx = -WORLD_WIDTH / 2; gx <= WORLD_WIDTH / 2; gx += GRID_SIZE)
    {
        AEMtx33Scale(&s, 2.0f, WORLD_HEIGHT);
        AEMtx33Trans(&t, gx, 0.0f);
        AEMtx33Concat(&transform, &t, &s);
        AEGfxSetTransform(transform.m);
        AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);
    }
    for (f32 gy = -WORLD_HEIGHT / 2; gy <= WORLD_HEIGHT / 2; gy += GRID_SIZE)
    {
        AEMtx33Scale(&s, WORLD_WIDTH, 2.0f);
        AEMtx33Trans(&t, 0.0f, gy);
        AEMtx33Concat(&transform, &t, &s);
        AEGfxSetTransform(transform.m);
        AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);
    }

    // AoE indicator
    f32    aoeRadius = 30.0f;
    AEVec2 playerPos = pPlayer->transform.position;
    AEMtx33Scale(&s, aoeRadius * 2.0f, aoeRadius * 2.0f);
    AEMtx33Trans(&t, playerPos.x, playerPos.y);
    AEMtx33Concat(&transform, &t, &s);
    AEGfxSetTransform(transform.m);
    AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.5f);
    AEGfxMeshDraw(Meshes::pCircleMesh, AE_GFX_MDM_TRIANGLES);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

    // Game gamePageObj
    DrawMinimap(gamePageObj, sCamX, sCamY);
    for (auto& obj : gamePageObj)
    {
        if (obj->ObjectType == NP)
        {
            NPC* npc = dynamic_cast<NPC*>(obj);
            if (!npc || !npc->isVisibleToPlayer) continue;
        }
        obj->Draw();
    }
    // Draw MiniMap
    //DrawTextRenderer(ammoText, { -500.0f , 400.0f }, 1.0f);

    // UI Elements
    UIelements.Draw(Meshes::pSquareLOriMesh);



}

// ============================================================================
// Game_Free
// ============================================================================
void Game_Free()
{
    waveSystem.Cleanup();

    for (auto& obj : gamePageObj) { delete obj; obj = nullptr; }
    gamePageObj.clear();
    gamePageObj.shrink_to_fit();
    //pPlayer = nullptr;
}

// ============================================================================
// Game_Unload
// ============================================================================
void Game_Unload()
{
    Meshes::FreeMeshes();
    AEGfxDestroyFont(gameFont);
    //AEGfxDestroyFont(gameFont); // <--- Add this to stop leaks!
    // UI Elements
    UIelements.Unload();
}