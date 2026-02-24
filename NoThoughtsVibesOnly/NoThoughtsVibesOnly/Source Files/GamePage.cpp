#include "pch.hpp"
#include "GamePage.hpp"
#include "StateManager.hpp"
#include "ExpUI.hpp"
#include "MiniMap.hpp"
#include "Player.hpp"
#include "NPC.hpp"
#include "GameObjectType.hpp"
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

// ---------------------------------------------------------------------------
// Camera
// ---------------------------------------------------------------------------
static f32    sCamX = 0.0f;
static f32    sCamY = 0.0f;
const  f32    CAM_SPEED = 5.0f;

// ============================================================================
// Game_Load
// ============================================================================
void Game_Load()
{
    Meshes::CreateTriangleMesh();
    Meshes::CreateSquareLeftOriginMesh();
    Meshes::CreateSquareCenterOriginMesh();
    Meshes::CreateCircleMesh();
    gameFont = AEGfxCreateFont("Assets/buggy-font.ttf", 30);
    LoadTextRenderer(ammoText, gameFont);
}

// ============================================================================
// Game_Init
// ============================================================================
void Game_Init()
{
    // Player
    pPlayer = new Player();
    sCamX = pPlayer->transform.position.x;
    sCamY = pPlayer->transform.position.y;

    InitTextRenderer(ammoText, "Ammo: 100%", { 1.0f,1.0f }, 1.0f, 1.0f, 1.0f);

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

    // ⭐ Health bars follow entities (world space, before camera reset)
    gameUI.DrawAllHealthBars();

    // -----------------------------------------------------------------------
    // SCREEN SPACE (camera reset to 0,0)
    // -----------------------------------------------------------------------
    AEGfxSetCamPosition(0.0f, 0.0f);

    // ⭐ All screen-space UI
    DrawTextRenderer(ammoText, { -500.0f, 400.0f }, 1.0f);
    gameUI.DrawHealthText();
    gameUI.DrawXPBar();
    gameUI.DrawCurrentStats();      // ← shows SPD / DMG / AOE at all times
    gameUI.DrawWaveInfo();
    gameUI.DrawWaveTimer();

    if (powerUpSystem.IsWaitingForUpgrade())
        gameUI.DrawPowerUpScreen();

    // Reset camera back to player
    AEGfxSetCamPosition(sCamX, sCamY);

    // -----------------------------------------------------------------------
    // PAUSE OVERLAY
    // -----------------------------------------------------------------------
    if (isPaused)
    {
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.7f);
        AEMtx33 pauseTransform, pauseScale, pauseTrans;
        AEMtx33Scale(&pauseScale, 3000.0f, 3000.0f);
        AEMtx33Trans(&pauseTrans, 0.0f, 0.0f);
        AEMtx33Concat(&pauseTransform, &pauseTrans, &pauseScale);
        AEGfxSetTransform(pauseTransform.m);
        AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

        TextRenderer pauseText;
        LoadTextRenderer(pauseText, gameFont);
        InitTextRenderer(pauseText, "PAUSED", { 1.0f, 1.0f }, 1.0f, 1.0f, 1.0f);
        DrawTextRenderer(pauseText, { 0.0f, 200.0f }, 3.0f);
        FreeTextRenderer(pauseText);

        TextRenderer hint;
        LoadTextRenderer(hint, gameFont);
        InitTextRenderer(hint, "ESC-Resume  R-Restart  Q-Menu", { 0.8f, 0.8f }, 0.8f, 0.8f, 1.0f);
        DrawTextRenderer(hint, { 0.0f, -50.0f }, 1.2f);
        FreeTextRenderer(hint);
    }
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
}