// ============================================================================
// GamePage.cpp - Main Gameplay State
// ============================================================================
// The core gameplay loop. Manages all in-game systems:
//
//   Player         : Input, movement, shooting, AoE, health
//   NPC / Enemies  : Updated individually via WaveSystem-spawned pool
//   Bullet Pool    : 500 player + 100 enemy pooled projectiles
//   WaveSystem     : Spawns progressive waves of enemies
//   PowerUpSystem  : XP, leveling, stat upgrades on wave clear
//   GameUI         : HUD, health bars, XP bar, wave info, power-up screen
//   Audio          : Background music + shoot SFX
//   Camera         : Lerp-follows the player
//   Minimap        : Top-right overlay showing all visible entities
//
// STATE FLOW:
// ----------------------------------------------------------------------------
//   Game_Load()   : Create meshes, fonts, audio
//   Game_Init()   : Create player, bullets, init systems
//   Game_Update() : Input, AI, physics, win/loss checks
//   Game_Draw()   : World geometry, entities, HUD, minimap
//   Game_Free()   : Destroy all GameObjects
//   Game_Unload() : Free meshes, fonts, audio
//
// KEY BINDINGS (in-game):
// ----------------------------------------------------------------------------
//   WASD        : Move player
//   QE          : Rotate player
//   LMB (hold)  : Shoot toward cursor
//   ESC         : Toggle pause overlay
//   R  (paused) : Restart
//   Q  (paused) : Return to main menu
//   C           : Start first wave
//   SPACE       : Jump to Finish (debug)
//   ENTER       : Jump to Win    (debug)
// ============================================================================

#include "pch.hpp"
#include "GamePage.hpp"
#include "StateManager.hpp"
#include "MiniMap.hpp"
#include "Player.hpp"
#include "NPC.hpp"
#include "GameObjectType.hpp"
#include <vector>
#include <iostream>
#include "Bullet.hpp"
#include "WaveSystem.hpp"
#include "PowerUpSystem.hpp"
#include "GameUI.hpp"
#include "Audio.hpp"
#include "Particles.hpp"
#include "LevelConfig.hpp"

// ============================================================================
// World constants
// ============================================================================
const f32 WORLD_WIDTH = 2000.0f;
const f32 WORLD_HEIGHT = 2000.0f;
const f32 GRID_SIZE = 100.0f;

// ============================================================================
// Global object lists
// ============================================================================
// gamePageObj - Every active GameObject in the game (player, enemies, bullets).
// Must be initialised before any GameObject constructor runs.
std::vector<GameObject*> gamePageObj;

// ============================================================================
// Global gameplay state
// ============================================================================
GameObject* pPlayer{ nullptr };   // Owning pointer to the player object
int availableBullets = 0;       // Count of inactive player bullets (shown in HUD)
static bool isPaused = false;   // True while the in-game pause overlay is shown

// ============================================================================
// Systems
// ============================================================================
PowerUpSystem powerUpSystem;    // XP, leveling, stat upgrades
WaveSystem    waveSystem;       // Enemy wave scheduling and spawning
GameUI        gameUI;           // All HUD rendering

// ============================================================================
// Font / HUD text
// ============================================================================
s8           gameFont = 0;      // Font handle reused across all HUD text
TextRenderer pauseText;
TextRenderer hint;
TextRenderer ammoText;          // "Ammo: X / 500" display
char         ammoBuffer[500];   // Scratch buffer for ammo string formatting

// ============================================================================
// Camera
// ============================================================================
static f32  sCamX = 0.0f;   // Current camera world X (lerp target = player X)
static f32  sCamY = 0.0f;   // Current camera world Y (lerp target = player Y)
const  f32  CAM_SPEED = 5.0f;  // Camera lerp speed (higher = snappier)

// ============================================================================
// Audio
// ============================================================================
Audio* bgMusic = nullptr;      // Background music (looping)
Audio* shootSFX = nullptr;      // Shoot sound effect
Audio* levelupSFX = nullptr;
bool levelUpSFXFlag = true;

// ============================================================================
// Game_Load
// ============================================================================
// Creates all mesh types, loads the font, and loads audio.
// Called ONCE when entering the playing state.
// ============================================================================
void Game_Load()
{
    Meshes::CreateTriangleMesh();
    Meshes::CreateSquareLeftOriginMesh();
    Meshes::CreateSquareCenterOriginMesh();
    Meshes::CreateCircleMesh();

    gameFont = AEGfxCreateFont("Assets/buggy-font.ttf", 30);

    // Audio: music loops forever (-1), SFX plays once (0)
    bgMusic = new Audio("Assets/Audio/BATTLE-MILITARY_GEN-HDF-03135.wav", -1, 0.5f, 1.0f, AudioType::MUSIC);
    shootSFX = new Audio("Assets/Audio/SCI-FI-LASER_GEN-HDF-20715.wav", 0, 1.0f, 1.0f, AudioType::SOUND);
    levelupSFX = new Audio("Assets/Audio/SCI-FI-POWER-UP_GEN-HDF-20769.wav", 0, 1.0f, 1.0f, AudioType::SOUND);
}

// ============================================================================
// Game_Init
// ============================================================================
// Allocates and starts all game objects. Called on every fresh entry
// (not on restart - restart skips Load/Unload but still calls Init/Free).
// ============================================================================
void Game_Init()
{

    // --- Player ---
    pPlayer = new Player();
    sCamX = pPlayer->transform.position.x;
    sCamY = pPlayer->transform.position.y;
	ammoText = TextRenderer(gameFont, 1.0f, { -500.0f, 400.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
	ammoText << "Ammo: 0 / 500"; // Initial text - updated dynamically in Game_Update

    // --- Systems ---
    waveSystem.Init(dynamic_cast<Player*>(pPlayer));
    waveSystem.SetLevelConfig(g_CurrentLevel);  // Apply selected level configuration
    powerUpSystem.Init();

    // Log level info
    std::cout << "\n==============================================\n"
        << "    LEVEL LOADED: ";
    switch (g_CurrentLevel.type)
    {
    case LevelType::LEVEL_1: std::cout << "Level 1 (5 waves, no boss)"; break;
    case LevelType::LEVEL_2: std::cout << "Level 2 (10 waves, boss on final wave)"; break;
    case LevelType::ENDLESS: std::cout << "Endless Mode"; break;
    }
    std::cout << "\n==============================================\n\n";

    // Link PowerUpSystem to player so it can query upgraded stats
    Player* player = dynamic_cast<Player*>(pPlayer);
    if (player) player->powerUpSystem = &powerUpSystem;

    gameUI.Init(gameFont, &waveSystem, &powerUpSystem);

    // --- Bullet pool: 500 player bullets ---
    for (int i = 0; i < 500; ++i)
    {
        Bullet* b = new Bullet();
        b->startPos = pPlayer;
        b->isActive = false;
        b->owner = BulletOwner::PLAYER;
        b->spriteRenderer.colour = { 1.0f, 1.0f, 0.0f, 0.0f }; // Hidden initially
    }

    // --- Bullet pool: 100 enemy bullets (startPos assigned by WaveSystem) ---
    for (int i = 0; i < 100; ++i)
    {
        Bullet* b = new Bullet();
        b->owner = BulletOwner::ENEMY;
        b->isActive = false;
        b->spriteRenderer.colour = { 1.0f, 0.0f, 0.0f, 0.0f };
    }

    // Start() all objects (sets initial transform, type, etc.)
    for (auto& obj : gamePageObj)
        obj->Start();

    if (bgMusic) bgMusic->Play();
}

// ============================================================================
// Game_Update
// ============================================================================
// Per-frame logic. Divided into:
//   1. Pause handling (ESC toggle, R/Q while paused)
//   2. Power-up selection screen (blocks gameplay while choosing)
//   3. Wave system update
//   4. Camera lerp
//   5. Available bullet count
//   6. Death / win condition checks
//   7. Update all game objects
//   8. HUD text refresh
//   9. Debug hotkeys
// ============================================================================
void Game_Update()
{
    // ------------------------------------------------------------------
    // 1. Pause toggle
    // ------------------------------------------------------------------
    if (AEInputCheckTriggered(AEVK_ESCAPE))
    {
        isPaused = !isPaused;
        if (isPaused) { if (bgMusic) bgMusic->Pause(); }
        else { if (bgMusic) bgMusic->Resume(); }
        return;
    }

    if (isPaused)
    {
        // Allow restart/quit while paused
        if (AEInputCheckTriggered(AEVK_R)) { isPaused = false; StateManagerChangeState(STATE_RESTART); return; }
        if (AEInputCheckTriggered(AEVK_Q)) { isPaused = false; StateManagerChangeState(STATE_MENU);    return; }
        return; // Freeze everything else while paused
    }

    const f32 dt = (f32)AEFrameRateControllerGetFrameTime();

    // ------------------------------------------------------------------
    // 2. Power-up selection (freezes the game until a choice is made)
    // ------------------------------------------------------------------
    if (powerUpSystem.IsWaitingForUpgrade())
    {
        PowerUp* choices = powerUpSystem.GetPowerUpChoices();
        Player* player = dynamic_cast<Player*>(pPlayer);
        if (levelupSFX && levelUpSFXFlag) { levelupSFX->Play(); levelUpSFXFlag = false; };
        if (AEInputCheckTriggered(AEVK_1)) powerUpSystem.ApplyPowerUp(choices[0].type, player);
        else if (AEInputCheckTriggered(AEVK_2)) powerUpSystem.ApplyPowerUp(choices[1].type, player);
        else if (AEInputCheckTriggered(AEVK_3)) powerUpSystem.ApplyPowerUp(choices[2].type, player);
        return;
    }
    else {
        levelUpSFXFlag = true; // Reset level-up SFX trigger for next time
    }

    // ------------------------------------------------------------------
    // 3. Wave system
    // ------------------------------------------------------------------
    waveSystem.Update(dt);

    // C key starts the very first wave manually
    if (AEInputCheckTriggered(AEVK_C) && waveSystem.GetCurrentWave() == 0)
        waveSystem.StartNextWave();

    // ------------------------------------------------------------------
    // 4. Camera: smooth lerp toward player position
    // ------------------------------------------------------------------
    sCamX += (pPlayer->transform.position.x - sCamX) * CAM_SPEED * dt;
    sCamY += (pPlayer->transform.position.y - sCamY) * CAM_SPEED * dt;

    // ------------------------------------------------------------------
    // 5. Count inactive player bullets for HUD ammo display
    // ------------------------------------------------------------------
    availableBullets = 0;
    for (auto& obj : gamePageObj)
    {
        if (obj->ObjectType == ObjectType::SHOT)
        {
            const Bullet* b = static_cast<Bullet*>(obj);
            if (b->owner == BulletOwner::PLAYER && !b->isActive)
                ++availableBullets;
        }
    }

    // ------------------------------------------------------------------
    // 6a. Player death check
    // ------------------------------------------------------------------
    if (dynamic_cast<Player*>(pPlayer)->health <= 0.0f)
    {
        if (bgMusic) bgMusic->Stop();
        StateManagerChangeState(STATE_FINISH);
        return;
    }

    // ------------------------------------------------------------------
    // 6b. Win condition: level complete (all waves cleared for non-endless)
    // ------------------------------------------------------------------
    if (waveSystem.IsLevelComplete())
    {
        if (bgMusic) bgMusic->Stop();
        StateManagerChangeState(STATE_WIN);
        return;
    }

    // ------------------------------------------------------------------
    // 7. Update all game objects
    // ------------------------------------------------------------------
    for (auto& obj : gamePageObj)
    {
        if (!obj) continue;

        if (obj->ObjectType == NP)
        {
            // NPCs have extra isActive/visibility guards inside Update()
            NPC* npc = dynamic_cast<NPC*>(obj);
            if (npc && obj->isActive) npc->Update(dt);
        }
        else
        {
            obj->Update(dt);
        }
    }

    // ------------------------------------------------------------------
    // 8. HUD ammo text refresh
    // ------------------------------------------------------------------
	//ammoText << "Ammo: " << availableBullets << " / 500";

    // ------------------------------------------------------------------
    // 9. Debug hotkeys
    // ------------------------------------------------------------------
    if (AEInputCheckTriggered(AEVK_SPACE))  StateManagerChangeState(STATE_FINISH);  // Force lose
    if (AEInputCheckTriggered(AEVK_RETURN)) StateManagerChangeState(STATE_WIN);      // Force win
    if (AEInputCheckTriggered(AEVK_R))      StateManagerChangeState(STATE_RESTART);
    if (AEInputCheckTriggered(AEVK_Q))      StateManagerChangeState(STATE_MENU);
}

// ============================================================================
// Game_Draw
// ============================================================================
// Rendering order:
//   WORLD SPACE (camera at player)
//     1. Red border (world edge indicator)
//     2. Grey floor
//     3. Grid lines
//     4. AoE indicator circle
//     5. Minimap
//     6. All active game objects (skips invisible NPCs)
//     7. Health bars (world-space, float above entities)
//   SCREEN SPACE (camera at 0,0)
//     8. Ammo text, HP text, XP bar, stats, wave info, wave timer
//     9. Power-up selection overlay (if leveled up)
//    10. Pause overlay (if paused)
// ============================================================================
void Game_Draw()
{
    // -----------------------------------------------------------------------
    // WORLD SPACE
    // -----------------------------------------------------------------------
    AEGfxSetCamPosition(sCamX, sCamY);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

    Transform tf; // Reused for all draw calls to avoid redundant matrix declarations

    // 1. World border (red outline)
    AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 1.0f);
    tf.SetPosition(0.0f, 0.0f);
    tf.SetScale(WORLD_WIDTH + 20.0f, WORLD_HEIGHT + 20.0f);
    tf.Apply();
    AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

    // 2. Floor
    AEGfxSetColorToMultiply(0.3f, 0.3f, 0.3f, 1.0f);
    tf.SetScale(WORLD_WIDTH, WORLD_HEIGHT);
    tf.Apply();
    AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

    // 3. Grid lines
    AEGfxSetColorToMultiply(0.5f, 0.5f, 0.5f, 1.0f);

    // Vertical lines
    tf.SetScale(2.0f, WORLD_HEIGHT);
    for (f32 gx = -WORLD_WIDTH / 2.0f; gx <= WORLD_WIDTH / 2.0f; gx += GRID_SIZE)
    {
        tf.SetPosition(gx, 0.0f);
        tf.Apply();
        AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);
    }

    // Horizontal lines
    tf.SetScale(WORLD_WIDTH, 2.0f);
    for (f32 gy = -WORLD_HEIGHT / 2.0f; gy <= WORLD_HEIGHT / 2.0f; gy += GRID_SIZE)
    {
        tf.SetPosition(0.0f, gy);
        tf.Apply();
        AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);
    }

    // 4. AoE radius indicator (semi-transparent circle around player)
    {
        // Cast the base GameObject pointer to Player to access specific stats
        Player* player = dynamic_cast<Player*>(pPlayer);
        if (player)
        {
            const f32    aoeRadius = player->GetAoeRadius();
            const AEVec2 playerPos = player->transform.position;

            tf.SetPosition(playerPos.x, playerPos.y);
            tf.SetUniformScale(aoeRadius);
            AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.5f);
            tf.Apply();
            AEGfxMeshDraw(Meshes::pCircleMesh, AE_GFX_MDM_TRIANGLES);
            AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
        }
    }

    // 5. Minimap
    DrawMinimap(gamePageObj, sCamX, sCamY);

    // 6. Game objects (skip invisible NPCs)
    for (auto& obj : gamePageObj)
    {
        if (obj->ObjectType == NP)
        {
            const NPC* npc = dynamic_cast<NPC*>(obj);
            if (!npc || !npc->isVisibleToPlayer) continue;
        }
        obj->Draw();
    }
    // Draw smoke on top of the player, before health bars
    Player* player = dynamic_cast<Player*>(pPlayer);
    if (player)
    {
        for (u8 idx : player->smokePS.GetActiveParticles())
            player->smokePS.Render(player->smokePS.GetParticles()[idx]);
    }
    // 7. World-space health bars (drawn before camera reset)
    gameUI.DrawAllHealthBars();

    // -----------------------------------------------------------------------
    // SCREEN SPACE
    // -----------------------------------------------------------------------
    AEGfxSetCamPosition(0.0f, 0.0f);

    // 8. HUD elements
    ammoText.Draw();
    gameUI.DrawHealthText();
    gameUI.DrawXPBar();
    gameUI.DrawCurrentStats();
    gameUI.DrawWaveInfo();
    gameUI.DrawWaveTimer();
    gameUI.DrawAbilities();

    // 9. Power-up overlay (blocks gameplay while active)
    if (powerUpSystem.IsWaitingForUpgrade())
        gameUI.DrawPowerUpScreen();
    
    if(AEInputCheckTriggered(AEVK_U))
		powerUpSystem.AddExperience(100.0f);

    // -----------------------------------------------------------------------
    // PAUSE OVERLAY (drawn in screen space so it covers everything)
    // -----------------------------------------------------------------------
    if (isPaused)
    {
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.7f);
        tf.SetPosition(0.0f, 0.0f);
        tf.SetUniformScale(3000.0f);
        tf.Apply();
        AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

        // "PAUSED" header
		pauseText = TextRenderer(gameFont, 1.0f, { 0.0f, 200.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
		pauseText << "PAUSED";
		pauseText.Draw();

        // Key hint text
		hint = TextRenderer(gameFont, 0.8f, { 0.0f, -50.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
		hint << "ESC-Resume  R-Restart  Q-Menu";
		hint.Draw();
    }

    // Restore world camera for next frame
    AEGfxSetCamPosition(sCamX, sCamY);
}

// ============================================================================
// Game_Free
// ============================================================================
// Destroys all game objects and clears the object list.
// Called before every state transition (including restart).
// ============================================================================
void Game_Free()
{
    waveSystem.Cleanup();

    for (auto& obj : gamePageObj)
    {
        delete obj;
        obj = nullptr;
    }
    gamePageObj.clear();
    gamePageObj.shrink_to_fit();

    //if (bgMusic) bgMusic->Free();
	//if (levelupSFX) levelupSFX->Free();
	//if (shootSFX) shootSFX->Free();
}

// ============================================================================
// Game_Unload
// ============================================================================
// Frees mesh and font resources, and deletes audio objects.
// Called only on a full state exit (NOT on restart).
// ============================================================================
void Game_Unload()
{
    Meshes::FreeMeshes();
    AEGfxDestroyFont(gameFont);
    //levelupSFX = nullptr;
    //bgMusic = nullptr;
    //shootSFX = nullptr;
    if (bgMusic) { bgMusic->Free();    delete bgMusic;    bgMusic = nullptr; }
    if (levelupSFX) { levelupSFX->Free(); delete levelupSFX; levelupSFX = nullptr; }
    if (shootSFX) { shootSFX->Free();   delete shootSFX;   shootSFX = nullptr; }
}