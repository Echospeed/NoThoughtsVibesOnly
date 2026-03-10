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
//   WASD             : Move player
//   Left/Right Arrow : Rotate player
//   LMB (hold)       : Shoot toward cursor
//   E                : Activate invulnerability ability
//   ESC              : Toggle pause overlay
//   TAB  (paused)    : Restart
//   Q  (paused)      : Return to main menu
//   C  (debug)       : Kill all active enemies instantly
//   U  (debug)       : Add 100 XP
//   SPACE (debug)    : Jump to Finish (lose) screen
//   ENTER (debug)    : Jump to Win screen
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

// ============================================================================
// Starfield - Two-layer parallax
// ============================================================================
struct Star
{
    f32 x, y;        // World position
    f32 size;        // Radius (1-3 units)
    f32 brightness;  // 0.4 - 1.0
    bool farLayer;   // true = far (0.8x speed), false = near (0.95x speed)
};

static const int   STAR_COUNT = 600;
static Star        s_Stars[STAR_COUNT];

static void GenerateStars()
{
    srand((unsigned int)AEGetTime(nullptr)); // Random seed - different stars every run
    for (int i = 0; i < STAR_COUNT; ++i)
    {
        s_Stars[i].x = ((f32)rand() / RAND_MAX) * WORLD_WIDTH - WORLD_WIDTH / 2.0f;
        s_Stars[i].y = ((f32)rand() / RAND_MAX) * WORLD_HEIGHT - WORLD_HEIGHT / 2.0f;
        s_Stars[i].brightness = 0.35f + ((f32)rand() / RAND_MAX) * 0.65f;

        // First 400 = far layer (tiny, dim), last 200 = near layer (slightly bigger, brighter)
        if (i < 400)
        {
            s_Stars[i].farLayer = true;
            s_Stars[i].size = 0.8f + ((f32)rand() / RAND_MAX) * 1.2f; // 0.8 - 2.0
            s_Stars[i].brightness *= 0.7f; // Dimmer in the distance
        }
        else
        {
            s_Stars[i].farLayer = false;
            s_Stars[i].size = 1.5f + ((f32)rand() / RAND_MAX) * 1.5f; // 1.5 - 3.0
        }
    }
}

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
// Game time accumulator (for animations like boss glow pulse)
// ============================================================================
static f32 s_GameTime = 0.0f;

// ============================================================================
// Screen Shake
// ============================================================================
static f32  s_ShakeTimer = 0.0f;   // Remaining shake duration (seconds)
static f32  s_ShakeMagnitude = 8.0f;   // Maximum pixel offset at full intensity
static f32  s_PrevPlayerHP = 100.0f; // HP last frame - used to detect damage

// ============================================================================
// Wave Announcement Banner
// ============================================================================
static f32  s_WaveAnnounceTimer = 0.0f;
static u32  s_WaveAnnounceNum = 0;
static u32  s_PrevWaveNum = 0;
static const f32 WAVE_ANNOUNCE_DURATION = 2.5f;

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

    NPC_LoadTextures(); // Load shared NPC texture cache once
    ParticleSystem::LoadSharedMesh(); // Shared mesh for all particle systems

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
    AEGfxSetBackgroundColor(0.02f, 0.02f, 0.05f);

    // Generate the fixed starfield for this session
    GenerateStars();

    // --- Player ---
    pPlayer = new Player();
    sCamX = pPlayer->transform.position.x;
    sCamY = pPlayer->transform.position.y;
    ammoText = TextRenderer(gameFont, 1.0f, { -500.0f, -400.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
    ammoText << "Ammo: 10 / 10"; // Updated dynamically in Game_Update

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

    // --- Bullet pool: 10 player bullets (grows by 5 per Bullet Damage upgrade) ---
    for (int i = 0; i < 10; ++i)
    {
        Bullet* b = new Bullet();
        b->startPos = pPlayer;
        b->isActive = false;
        b->owner = BulletOwner::PLAYER;
        b->spriteRenderer.colour = { 1.0f, 1.0f, 0.0f, 0.0f }; // Hidden initially
    }

    // --- Enemy bullet pool ---
    // Max alive simultaneously: ~7 Rangers (x3 each) + 1 Boss (x8) = 29.
    // 60 gives safe headroom. Slots start unassigned (startPos == nullptr).
    // WaveSystem assigns them on NPC spawn; NPC clears them on death.
    for (int i = 0; i < 60; ++i)
    {
        Bullet* b = new Bullet();
        b->owner = BulletOwner::ENEMY;
        b->isActive = false;
        b->spriteRenderer.colour = { 1.0f, 0.0f, 0.0f, 0.0f }; // Hidden
    }

    // Start() all objects (sets initial transform, type, etc.)
    for (auto& obj : gamePageObj)
        obj->Start();

    if (bgMusic) bgMusic->Play();

    // Reset visual polish state
    s_GameTime = 0.0f;
    s_ShakeTimer = 0.0f;
    s_PrevPlayerHP = 100.0f;
    s_WaveAnnounceTimer = 0.0f;
    s_WaveAnnounceNum = 0;
    s_PrevWaveNum = 0;

    // Auto-start the first wave immediately
    waveSystem.StartNextWave();
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
        if (AEInputCheckTriggered(AEVK_TAB)) { isPaused = false; StateManagerChangeState(STATE_RESTART); return; }
        if (AEInputCheckTriggered(AEVK_Q)) { isPaused = false; StateManagerChangeState(STATE_MENU);    return; }
        return; // Freeze everything else while paused
    }

    const f32 dt = (f32)AEFrameRateControllerGetFrameTime();
    s_GameTime += dt; // Drives all time-based animations (boss glow, border pulse)

    // ------------------------------------------------------------------
    // 2. Power-up selection (freezes the game until a choice is made)
    // ------------------------------------------------------------------
    if (powerUpSystem.IsWaitingForUpgrade())
    {
        const PowerUp* choices = powerUpSystem.GetPowerUpChoices();
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

    // C key kills all active enemies (debug/cheat)
    if (AEInputCheckTriggered(AEVK_C))
    {
        for (auto& obj : gamePageObj)
        {
            if (obj && obj->ObjectType == NP && obj->isActive)
            {
                NPC* npc = dynamic_cast<NPC*>(obj);
                if (npc)
                {
                    npc->health = 0.0f;  // Kill the enemy
                    std::cout << "[DEBUG] Killed NPC\n";
                }
            }
        }
    }

    // ------------------------------------------------------------------
    // 4. Camera: smooth lerp toward player position
    // ------------------------------------------------------------------
    sCamX += (pPlayer->transform.position.x - sCamX) * CAM_SPEED * dt;
    sCamY += (pPlayer->transform.position.y - sCamY) * CAM_SPEED * dt;

    // Wave announcement banner
    {
        const u32 curWave = waveSystem.GetCurrentWave();
        if (curWave != s_PrevWaveNum && curWave > 0)
        {
            s_WaveAnnounceNum = curWave;
            s_WaveAnnounceTimer = WAVE_ANNOUNCE_DURATION;
            s_PrevWaveNum = curWave;
        }
        if (s_WaveAnnounceTimer > 0.0f) s_WaveAnnounceTimer -= dt;
    }

    // Screen shake: trigger when player takes damage this frame
    {
        Player* p = dynamic_cast<Player*>(pPlayer);
        if (p && p->health < s_PrevPlayerHP)
            s_ShakeTimer = 0.2f;
        if (p) s_PrevPlayerHP = p->health;
    }
    if (s_ShakeTimer > 0.0f) s_ShakeTimer -= dt;

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
    {
        Player* p = dynamic_cast<Player*>(pPlayer);
        if (p && p->health <= 0.0f)
        {
            if (bgMusic) bgMusic->Stop();
            StateManagerChangeState(STATE_FINISH);
            return;
        }
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
            if (npc) npc->Update(dt);
        }
        else
        {
            obj->Update(dt);
        }
    }

    // ------------------------------------------------------------------
    // 8. HUD ammo text refresh
    // ------------------------------------------------------------------
    {
        u32 totalBullets = powerUpSystem.GetStats().bulletCount;
        Player* p = dynamic_cast<Player*>(pPlayer);
        ammoText = TextRenderer(gameFont, 1.0f, { -500.0f, -400.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
        if (p && p->IsReloading())
            ammoText << "RELOADING...";
        else
            ammoText << "Ammo: " << (p ? p->GetAmmoInMagazine() : 0) << " / " << totalBullets;
    }

    // ------------------------------------------------------------------
    // 9. Debug hotkeys
    // ------------------------------------------------------------------
    if (AEInputCheckTriggered(AEVK_SPACE))  StateManagerChangeState(STATE_FINISH);  // Force lose
    if (AEInputCheckTriggered(AEVK_RETURN)) StateManagerChangeState(STATE_WIN);      // Force win
    if (AEInputCheckTriggered(AEVK_TAB))      StateManagerChangeState(STATE_RESTART);
    if (AEInputCheckTriggered(AEVK_Q))      StateManagerChangeState(STATE_MENU);
}

// ============================================================================
// Game_Draw
// ============================================================================
// Rendering order:
//   WORLD SPACE (camera at player)
//     1. Deep red world border (pulsing glow, intensifies near edge)
//     2. Deep space floor (near-black blue)
//     3. Stars (600 circle dots, two parallax layers)
//     4. AoE indicator circle (around player)
//     5. Entity glow halos (player, boss, bullets - drawn before entities)
//     6. All active game objects (skips invisible NPCs)
//     7. Particle effects (smoke, explosions)
//     8. Health bars (world-space, float above entities)
//     9. Minimap (top-right overlay)
//   SCREEN SPACE (camera at 0,0)
//    10. Ammo text, HP text, XP bar, stats, wave info, wave timer, abilities
//    11. Power-up selection overlay (if leveled up)
//    12. Wave announcement banner (fades in/out on new wave)
//    13. Pause overlay (if paused)
// ============================================================================
void Game_Draw()
{
    // -----------------------------------------------------------------------
    // WORLD SPACE
    // -----------------------------------------------------------------------
    // Screen shake: offset camera by a random amount each frame while active.
    f32 shakeX = 0.0f, shakeY = 0.0f;
    if (s_ShakeTimer > 0.0f)
    {
        const f32 intensity = s_ShakeTimer / 0.2f;
        shakeX = (((f32)rand() / RAND_MAX) * 2.0f - 1.0f) * s_ShakeMagnitude * intensity;
        shakeY = (((f32)rand() / RAND_MAX) * 2.0f - 1.0f) * s_ShakeMagnitude * intensity;
    }
    AEGfxSetCamPosition(sCamX + shakeX, sCamY + shakeY);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

    Transform tf;

    // 1. World border - pulsing red glow, intensifies near the edge
    {
        const Player* pp = dynamic_cast<Player*>(pPlayer);
        const f32 px = pp ? pp->transform.position.x : 0.0f;
        const f32 py = pp ? pp->transform.position.y : 0.0f;
        const f32 halfW = WORLD_WIDTH / 2.0f;
        const f32 halfH = WORLD_HEIGHT / 2.0f;

        const f32 pulse = 0.5f + 0.5f * sinf(s_GameTime * 2.0f);
        const f32 distX = halfW - fabsf(px);
        const f32 distY = halfH - fabsf(py);
        const f32 nearestEdge = (distX < distY) ? distX : distY;
        const f32 distToEdge = (nearestEdge < 300.0f) ? nearestEdge : 300.0f;
        const f32 edgeProximity = 1.0f - AEClamp(distToEdge / 300.0f, 0.0f, 1.0f);
        const f32 glowStrength = 0.35f + pulse * 0.15f + edgeProximity * 0.5f;

        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);

        // Outer glow layer
        AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.0f);
        AEGfxSetColorToAdd(glowStrength, 0.02f, 0.02f, 0.6f);
        tf.SetPosition(0.0f, 0.0f);
        tf.SetScale(WORLD_WIDTH + 60.0f, WORLD_HEIGHT + 60.0f);
        tf.Apply();
        AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

        // Inner border
        AEGfxSetColorToAdd(glowStrength * 0.7f, 0.02f, 0.02f, 1.0f);
        tf.SetScale(WORLD_WIDTH + 20.0f, WORLD_HEIGHT + 20.0f);
        tf.Apply();
        AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);
    }

    // 2. Deep space floor
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxSetColorToAdd(0.03f, 0.03f, 0.07f, 1.0f);
    tf.SetPosition(0.0f, 0.0f);
    tf.SetScale(WORLD_WIDTH, WORLD_HEIGHT);
    tf.Apply();
    AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

    // 3. Stars - two parallax layers
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    for (int i = 0; i < STAR_COUNT; ++i)
    {
        const Star& s = s_Stars[i];
        const f32 parallaxFactor = s.farLayer ? 0.80f : 0.95f;
        const f32 warmth = (i % 3 == 0) ? 0.85f : 1.0f;
        const f32 drawX = s.x - sCamX * (1.0f - parallaxFactor);
        const f32 drawY = s.y - sCamY * (1.0f - parallaxFactor);
        AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.0f);
        AEGfxSetColorToAdd(s.brightness * warmth, s.brightness * warmth, s.brightness, 1.0f);
        tf.SetPosition(drawX, drawY);
        tf.SetUniformScale(s.size);
        tf.Apply();
        AEGfxMeshDraw(Meshes::pCircleMesh, AE_GFX_MDM_TRIANGLES);
    }

    // Reset colour state
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    // 5. AoE radius indicator
    {
        Player* player = dynamic_cast<Player*>(pPlayer);
        if (player)
        {
            AEGfxSetRenderMode(AE_GFX_RM_COLOR);
            AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.0f);
            AEGfxSetColorToAdd(0.05f, 0.30f, 0.30f, 0.25f);
            AEGfxSetBlendMode(AE_GFX_BM_BLEND);
            tf.SetPosition(player->transform.position.x, player->transform.position.y);
            tf.SetUniformScale(player->GetAoeRadius());
            tf.Apply();
            AEGfxMeshDraw(Meshes::pCircleMesh, AE_GFX_MDM_TRIANGLES);
            AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
            AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
        }
    }

    // ============================================================================
    // Entity Glow Halos - drawn BEFORE entities so glows sit visually behind them
    // ============================================================================
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.0f);
    for (auto& obj : gamePageObj)
    {
        if (!obj->isActive) continue;

        if (obj == pPlayer)
        {
            // Player: dark red square halo
            AEGfxSetColorToAdd(0.6f, 0.0f, 0.0f, 0.18f);
            tf.SetPosition(obj->transform.position.x, obj->transform.position.y);
            tf.SetScale(obj->transform.scale.x * 1.7f, obj->transform.scale.y * 1.7f);
            tf.Apply();
            AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);
        }
        else if (obj->ObjectType == NP)
        {
            const NPC* npc = dynamic_cast<NPC*>(obj);
            if (!npc || !npc->isVisibleToPlayer) continue;
            if (npc->type == NPC_BOSS)
            {
                // Boss: pulsing purple halo
                const f32 bossPulse = 0.5f + 0.5f * sinf(s_GameTime * 3.0f);
                const f32 bossScale = 1.6f + bossPulse * 0.3f;
                AEGfxSetColorToAdd(0.5f, 0.0f, 0.8f, 0.12f + bossPulse * 0.1f);
                tf.SetPosition(obj->transform.position.x, obj->transform.position.y);
                tf.SetScale(obj->transform.scale.x * bossScale, obj->transform.scale.y * bossScale);
                tf.Apply();
                AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);
            }
        }
        else if (obj->ObjectType == SHOT)
        {
            const Bullet* b = dynamic_cast<Bullet*>(obj);
            if (!b) continue;
            const f32 bulletSize = obj->transform.scale.x;
            if (b->owner == BulletOwner::PLAYER)
                AEGfxSetColorToAdd(1.0f, 0.9f, 0.0f, 0.3f); // Yellow
            else
                AEGfxSetColorToAdd(1.0f, 0.1f, 0.0f, 0.3f); // Red
            tf.SetPosition(obj->transform.position.x, obj->transform.position.y);
            tf.SetUniformScale(bulletSize * 2.8f);
            tf.Apply();
            AEGfxMeshDraw(Meshes::pCircleMesh, AE_GFX_MDM_TRIANGLES);
        }
    }

    // Reset colour state before drawing actual entities
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    // 7. Game objects (skip invisible NPCs)
    for (auto& obj : gamePageObj)
    {
        if (obj->ObjectType == NP)
        {
            const NPC* npc = dynamic_cast<NPC*>(obj);
            if (!npc || !npc->isVisibleToPlayer) continue;
        }
        obj->Draw();
    }

    // Particles
    Player* player = dynamic_cast<Player*>(pPlayer);
    if (player)
    {
        player->smokePS.Render();
        for (GameObject* obj : gamePageObj)
        {
            if (obj->ObjectType != NP) continue;
            NPC* npc = dynamic_cast<NPC*>(obj);
            if (npc && npc->hasExploded)
                npc->explosionParticles.Render();
        }
    }

    // 8. World-space health bars
    gameUI.DrawAllHealthBars();

    // 6. Minimap
    DrawMinimap(gamePageObj, sCamX, sCamY);

    // Restore camera (undo shake for next frame)
    AEGfxSetCamPosition(sCamX, sCamY);

    // -----------------------------------------------------------------------
    // SCREEN SPACE
    // -----------------------------------------------------------------------
    AEGfxSetCamPosition(0.0f, 0.0f);

    // 9. HUD elements
    ammoText.Draw();
    gameUI.DrawHealthText();
    gameUI.DrawXPBar();
    gameUI.DrawCurrentStats();
    gameUI.DrawWaveInfo();
    gameUI.DrawWaveTimer();
    gameUI.DrawAbilities();

    // 10. Power-up overlay
    if (powerUpSystem.IsWaitingForUpgrade())
        gameUI.DrawPowerUpScreen();

    if (AEInputCheckTriggered(AEVK_U))
        powerUpSystem.AddExperience(100.0f);

    // ============================================================================
    // Wave Announcement Banner
    // Suppressed while the power-up overlay is open so it doesn't draw on top of the cards
    // ============================================================================
    if (s_WaveAnnounceTimer > 0.0f && !powerUpSystem.IsWaitingForUpgrade())
    {
        const f32 t = s_WaveAnnounceTimer / WAVE_ANNOUNCE_DURATION; // 1.0 -> 0.0
        f32 alpha;
        if (t > 0.8f) alpha = (1.0f - t) / 0.2f;
        else if (t > 0.4f) alpha = 1.0f;
        else               alpha = t / 0.4f;

        const f32 scale = 1.0f + (t > 0.8f ? (t - 0.8f) * 2.0f : 0.0f);

        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.0f);
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, alpha * 0.5f);
        tf.SetPosition(0.0f, 50.0f);
        tf.SetScale(500.0f * scale, 80.0f * scale);
        tf.Apply();
        AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

        TextRenderer waveBanner(gameFont, 1.5f * scale, { 0.0f, 35.0f }, { 1.0f, 0.3f, 0.3f, alpha });
        waveBanner << "WAVE " << s_WaveAnnounceNum;
        waveBanner.Draw();
    }

    // -----------------------------------------------------------------------
    // PAUSE OVERLAY
    // -----------------------------------------------------------------------
    if (isPaused)
    {
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.7f);
        tf.SetPosition(0.0f, 0.0f);
        tf.SetUniformScale(3000.0f);
        tf.Apply();
        AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

        pauseText = TextRenderer(gameFont, 1.0f, { 0.0f, 200.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
        pauseText << "PAUSED";
        pauseText.Draw();

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
    ParticleSystem::FreeSharedMesh(); // Release shared particle mesh
    NPC_UnloadTextures(); // Release shared NPC texture cache
    AEGfxDestroyFont(gameFont);
    //levelupSFX = nullptr;
    //bgMusic = nullptr;
    //shootSFX = nullptr;
    if (bgMusic) { bgMusic->Free();    delete bgMusic;    bgMusic = nullptr; }
    if (levelupSFX) { levelupSFX->Free(); delete levelupSFX; levelupSFX = nullptr; }
    if (shootSFX) { shootSFX->Free();   delete shootSFX;   shootSFX = nullptr; }
}