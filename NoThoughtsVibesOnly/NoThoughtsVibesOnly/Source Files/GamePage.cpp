// ============================================================================
// GamePage.cpp - Main Gameplay State
// ============================================================================
// (unchanged sections omitted for brevity - only Game_Init and score sections
//  are modified; all other functions are identical to the original)
//
// CHANGES vs original:
//   - Player bullet pool size: GameConfig::Gameplay().playerBulletPoolSize
//                              (was hardcoded 10)
//   - Enemy bullet pool size:  GameConfig::Gameplay().enemyBulletPoolSize
//                              (was hardcoded 60)
//   - Score per wave (normal): GameConfig::Gameplay().scorePerWaveNormal
//                              (was hardcoded 100)
//   - Score per wave (endless):GameConfig::Gameplay().scorePerWaveEndless
//                              (was hardcoded 150)
// ============================================================================

#include "pch.hpp"
#include "GamePage.hpp"
#include "StateManager.hpp"
#include "MiniMap.hpp"
#include "Player.hpp"
#include "NPC.hpp"
#include "GameObjectType.hpp"
#include <vector>
#include "Bullet.hpp"
#include "WaveSystem.hpp"
#include "PowerUpSystem.hpp"
#include "GameUI.hpp"
#include "Particles.hpp"
#include "LevelConfig.hpp"
#include "PausePage.hpp"
#include <random>

// ============================================================================
// World constants
// ============================================================================
f32 WORLD_WIDTH = 2500.0f;
f32 WORLD_HEIGHT = 2500.0f;

// ============================================================================
// Starfield - Two-layer parallax
// ============================================================================
struct Star
{
    f32 x, y;
    f32 size;
    f32 brightness;
    bool farLayer;
};
//============================================================================
// Generate a fixed array of stars with random positions, sizes, and brightness.
// Stars in the far layer are dimmer and smaller to enhance the parallax effect.
//============================================================================
static const int STAR_COUNT = 600;
static Star      s_Stars[STAR_COUNT];

static void GenerateStars()
{
    std::mt19937 rng(static_cast<unsigned int>(AEGetTime(nullptr)));
    std::uniform_real_distribution<f32> distX(-WORLD_WIDTH / 2.0f, WORLD_WIDTH / 2.0f);
    std::uniform_real_distribution<f32> distY(-WORLD_HEIGHT / 2.0f, WORLD_HEIGHT / 2.0f);
    std::uniform_real_distribution<f32> distBright(0.35f, 1.0f);
    std::uniform_real_distribution<f32> distSizeNear(1.5f, 3.0f);
    std::uniform_real_distribution<f32> distSizeFar(0.8f, 2.0f);

    for (int i = 0; i < STAR_COUNT; ++i)
    {
        s_Stars[i].x = distX(rng);
        s_Stars[i].y = distY(rng);
        s_Stars[i].brightness = distBright(rng);

        if (i < 400)
        {
            s_Stars[i].farLayer = true;
            s_Stars[i].size = distSizeFar(rng);
            s_Stars[i].brightness *= 0.7f;
        }
        else
        {
            s_Stars[i].farLayer = false;
            s_Stars[i].size = distSizeNear(rng);
        }
    }
}

// ============================================================================
// Global object lists
// ============================================================================
std::vector<GameObject*> gamePageObj;

// ============================================================================
// Global gameplay state
// ============================================================================
GameObject* pPlayer{ nullptr };
int  availableBullets = 0;
bool isPaused = false;

// ============================================================================
// Systems
// ============================================================================
PowerUpSystem powerUpSystem;
WaveSystem    waveSystem;
GameUI        gameUI;

// ============================================================================
// Font / HUD text
// ============================================================================
s8           gameFont = 0;
TextRenderer ammoText;
char         ammoBuffer[500];

// ============================================================================
// Camera
// ============================================================================
static f32 sCamX = 0.0f;
static f32 sCamY = 0.0f;
const  f32 CAM_SPEED = 5.0f;

// ============================================================================
// Game time accumulator
// ============================================================================
static f32 s_GameTime = 0.0f;

// ============================================================================
// Final score snapshot
// ============================================================================
int g_FinalScore = 0;
int g_FinalWaveCount = 0;

// ============================================================================
// Screen Shake
// ============================================================================
static f32 s_ShakeTimer = 0.0f;
static f32 s_ShakeMagnitude = 8.0f;
static f32 s_PrevPlayerHP = 100.0f;

// ============================================================================
// Wave Announcement Banner
// ============================================================================
static f32  s_WaveAnnounceTimer = 0.0f;
static u32  s_WaveAnnounceNum = 0;
static u32  s_PrevWaveNum = 0;
static const f32 WAVE_ANNOUNCE_DURATION = 2.5f;

bool levelUpSFXFlag = true;

// ============================================================================
// Game_Load
// ============================================================================
void Game_Load()
{
    GameConfig::Load(); // Load all JSON configs before anything else

    Meshes::CreateTriangleMesh();
    Meshes::CreateSquareLeftOriginMesh();
    Meshes::CreateSquareCenterOriginMesh();
    Meshes::CreateCircleMesh();

    NPC_LoadTextures(); // Uses texture paths from npc_config.json

    PausePage_Load();

    gameFont = AEGfxCreateFont("Assets/buggy-font.ttf", 30);
}

// ============================================================================
// Game_Init
// ============================================================================
void Game_Init()
{
    AEGfxSetBackgroundColor(0.02f, 0.02f, 0.05f);

    WORLD_WIDTH = GameConfig::Gameplay().world.width;
    WORLD_HEIGHT = GameConfig::Gameplay().world.height;

    GenerateStars();

    // --- Player ---
    pPlayer = new Player();
    sCamX = pPlayer->transform.position.x;
    sCamY = pPlayer->transform.position.y;
    ammoText = TextRenderer(gameFont, 1.0f, { -500.0f, -400.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
    ammoText << "Ammo: 10 / 10";

    // --- Systems ---
    waveSystem.Init(dynamic_cast<Player*>(pPlayer));
    waveSystem.SetLevelConfig(g_CurrentLevel);
    powerUpSystem.Init();

    //std::cout << "\n==============================================\n"
    //    << "    LEVEL LOADED: ";
    //switch (g_CurrentLevel.type)
    //{
    //case LevelType::LEVEL_1: std::cout << "Level 1 (5 waves, no boss)";           break;
    //case LevelType::LEVEL_2: std::cout << "Level 2 (10 waves, boss on final wave)"; break;
    //case LevelType::ENDLESS: std::cout << "Endless Mode";                           break;
    //}
    //std::cout << "\n==============================================\n\n";

    Player* player = dynamic_cast<Player*>(pPlayer);
    if (player) player->powerUpSystem = &powerUpSystem;

    gameUI.Init(gameFont, &waveSystem, &powerUpSystem);

    // --- Bullet pools: sizes from gameplay_config.json ---
    const int playerPoolSize = GameConfig::Gameplay().playerBulletPoolSize; // was hardcoded 10
    const int enemyPoolSize = GameConfig::Gameplay().enemyBulletPoolSize;  // was hardcoded 60

    for (int i = 0; i < playerPoolSize; ++i)
    {
        Bullet* b = new Bullet();
        b->startPos = pPlayer;
        b->isActive = false;
        b->owner = BulletOwner::PLAYER;
        b->spriteRenderer.colour = { 1.0f, 1.0f, 0.0f, 0.0f };
    }

    for (int i = 0; i < enemyPoolSize; ++i)
    {
        Bullet* b = new Bullet();
        b->owner = BulletOwner::ENEMY;
        b->isActive = false;
        b->spriteRenderer.colour = { 1.0f, 0.0f, 0.0f, 0.0f };
    }

    for (auto& obj : gamePageObj)
        obj->Start();

    AudioManager::PlayMusic("GameMusic");

    s_GameTime = 0.0f;
    s_ShakeTimer = 0.0f;
    s_PrevPlayerHP = 100.0f;
    s_WaveAnnounceTimer = 0.0f;
    s_WaveAnnounceNum = 0;
    s_PrevWaveNum = 0;

    waveSystem.StartNextWave();

    PausePage_Init();
}

// ============================================================================
// Game_Update
// ============================================================================
void Game_Update()
{
    if (AEInputCheckTriggered(AEVK_ESCAPE))
    {
        isPaused = !isPaused;
        if (isPaused) { AudioManager::PauseMusic("GameMusic"); }
        else
        {
            AudioManager::ResumeMusic("GameMusic");
            Player* p = dynamic_cast<Player*>(pPlayer);
            if (p) p->suppressShootOneFrame = true;
        }
        return;
    }

    if (isPaused) return;

    const f32 dt = static_cast<f32>(AEFrameRateControllerGetFrameTime());
    s_GameTime += dt;

    // Power-up selection
    if (powerUpSystem.IsWaitingForUpgrade())
    {
        const PowerUp* choices = powerUpSystem.GetPowerUpChoices();
        Player* player = dynamic_cast<Player*>(pPlayer);
        if (levelUpSFXFlag) { AudioManager::PlaySFX("LevelUp"); levelUpSFXFlag = false; AudioManager::PauseMusic("GameMusic");}
        if (AEInputCheckTriggered(AEVK_1)) powerUpSystem.ApplyPowerUp(choices[0].type, player);
        else if (AEInputCheckTriggered(AEVK_2)) powerUpSystem.ApplyPowerUp(choices[1].type, player);
        else if (AEInputCheckTriggered(AEVK_3)) powerUpSystem.ApplyPowerUp(choices[2].type, player);
        return;
    }
    else { levelUpSFXFlag = true;  AudioManager::ResumeMusic("GameMusic"); }

    waveSystem.Update(dt);

    if (AEInputCheckTriggered(AEVK_C))
    {
        for (auto& obj : gamePageObj)
        {
            if (obj && obj->ObjectType == NP && obj->isActive)
            {
                NPC* npc = dynamic_cast<NPC*>(obj);
                if (npc) { npc->health = 0.0f; } //std::cout << "[DEBUG] Killed NPC\n"; }
            }
        }
    }

    // Camera
    const f32 CAM_DEADZONE_RADIUS = 150.0f;
    f32 dx = pPlayer->transform.position.x - sCamX;
    f32 dy = pPlayer->transform.position.y - sCamY;
    f32 dist = sqrtf(dx * dx + dy * dy);
    if (dist > CAM_DEADZONE_RADIUS)
    {
        f32 excess = dist - CAM_DEADZONE_RADIUS;
        sCamX += (dx / dist * excess) * CAM_SPEED * dt;
        sCamY += (dy / dist * excess) * CAM_SPEED * dt;
    }

    f32 halfScreenWidth = (AEGfxGetWinMaxX() - AEGfxGetWinMinX()) / 2.0f;
    f32 halfScreenHeight = (AEGfxGetWinMaxY() - AEGfxGetWinMinY()) / 2.0f;
    sCamX = AEClamp(sCamX, -(WORLD_WIDTH / 2.0f) + halfScreenWidth - 30.0f, (WORLD_WIDTH / 2.0f) - halfScreenWidth + 30.0f);
    sCamY = AEClamp(sCamY, -(WORLD_HEIGHT / 2.0f) + halfScreenHeight - 30.0f, (WORLD_HEIGHT / 2.0f) - halfScreenHeight + 30.0f);

    // Wave announcement
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

    // Screen shake on damage
    {
        Player* p = dynamic_cast<Player*>(pPlayer);
        if (p && p->health < s_PrevPlayerHP) s_ShakeTimer = 0.2f;
        if (p) s_PrevPlayerHP = p->health;
    }
    if (s_ShakeTimer > 0.0f) s_ShakeTimer -= dt;

    // Available bullet count
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

    // Player death check
    {
        Player* p = dynamic_cast<Player*>(pPlayer);
        if (p && p->health <= 0.0f)
        {
            // Score per wave from config - was hardcoded 150/100
            const int scorePerWave = (g_CurrentLevel.type == LevelType::ENDLESS)
                ? GameConfig::Gameplay().scorePerWaveEndless
                : GameConfig::Gameplay().scorePerWaveNormal;

            g_FinalWaveCount = (g_CurrentLevel.type == LevelType::ENDLESS)
                ? static_cast<s8>(waveSystem.GetCurrentRound())
                : static_cast<s8>(waveSystem.GetCurrentWave());
            g_FinalScore = g_FinalWaveCount * scorePerWave;

            AudioManager::StopMusic("GameMusic");
            StateManagerChangeState(STATE_FINISH);
            return;
        }
    }

    // Win condition
    if (waveSystem.IsLevelComplete())
    {
        const int scorePerWave = (g_CurrentLevel.type == LevelType::ENDLESS)
            ? GameConfig::Gameplay().scorePerWaveEndless
            : GameConfig::Gameplay().scorePerWaveNormal;

        g_FinalWaveCount = (g_CurrentLevel.type == LevelType::ENDLESS)
            ? static_cast<s8>(waveSystem.GetCurrentRound())
            : static_cast<s8>(waveSystem.GetCurrentWave());
        g_FinalScore = g_FinalWaveCount * scorePerWave;

        AudioManager::StopMusic("GameMusic");
        StateManagerChangeState(STATE_WIN);
        return;
    }

    // Update all game objects
    for (auto& obj : gamePageObj)
    {
        if (!obj) continue;
        if (obj->ObjectType == NP)
        {
            NPC* npc = dynamic_cast<NPC*>(obj);
            if (npc) npc->Update(dt);
        }
        else { obj->Update(dt); }
    }

    // HUD ammo text refresh
    {
        u32 totalBullets = powerUpSystem.GetStats().bulletCount;
        Player* p = dynamic_cast<Player*>(pPlayer);
        ammoText = TextRenderer(gameFont, 1.0f, { -500.0f, -400.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
        if (p && p->IsReloading())
            ammoText << "RELOADING...";
        else
            ammoText << "Ammo: " << (p ? p->GetAmmoInMagazine() : 0) << " / " << totalBullets;
    }

    // Debug hotkeys
    if (AEInputCheckTriggered(AEVK_SPACE))  StateManagerChangeState(STATE_FINISH);
    if (AEInputCheckTriggered(AEVK_RETURN)) StateManagerChangeState(STATE_WIN);
    if (AEInputCheckTriggered(AEVK_TAB))    StateManagerChangeState(STATE_RESTART);
    if (AEInputCheckTriggered(AEVK_Q))      StateManagerChangeState(STATE_MENU);
}

// ============================================================================
// Game_Draw
// ============================================================================
void Game_Draw()
{
    // Screen shake
    f32 shakeX = 0.0f, shakeY = 0.0f;
    if (s_ShakeTimer > 0.0f)
    {
        const f32 intensity = s_ShakeTimer / 0.2f;
        static std::mt19937 shakeRng(std::random_device{}());
        std::uniform_real_distribution<f32> shakeDist(-1.0f, 1.0f);
        shakeX = shakeDist(shakeRng) * s_ShakeMagnitude * intensity;
        shakeY = shakeDist(shakeRng) * s_ShakeMagnitude * intensity;
    }
    AEGfxSetCamPosition(sCamX + shakeX, sCamY + shakeY);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

    Transform tf;

    // 1. World border
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
        const f32 edgeProx = 1.0f - AEClamp(distToEdge / 300.0f, 0.0f, 1.0f);
        const f32 glow = 0.35f + pulse * 0.15f + edgeProx * 0.5f;

        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.0f);
        AEGfxSetColorToAdd(glow, 0.02f, 0.02f, 0.6f);
        tf.SetPosition(0.0f, 0.0f);
        tf.SetScale(WORLD_WIDTH + 60.0f, WORLD_HEIGHT + 60.0f);
        tf.Apply();
        AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

        AEGfxSetColorToAdd(glow * 0.7f, 0.02f, 0.02f, 1.0f);
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

    // 3. Stars
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    for (int i = 0; i < STAR_COUNT; ++i)
    {
        const Star& s = s_Stars[i];
        const f32 pFactor = s.farLayer ? 0.80f : 0.95f;
        const f32 warmth = (i % 3 == 0) ? 0.85f : 1.0f;
        const f32 drawX = s.x - sCamX * (1.0f - pFactor);
        const f32 drawY = s.y - sCamY * (1.0f - pFactor);
        AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.0f);
        AEGfxSetColorToAdd(s.brightness * warmth, s.brightness * warmth, s.brightness, 1.0f);
        tf.SetPosition(drawX, drawY);
        tf.SetUniformScale(s.size);
        tf.Apply();
        AEGfxMeshDraw(Meshes::pCircleMesh, AE_GFX_MDM_TRIANGLES);
    }

    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    // 4. AoE radius indicator
    {
        Player* player = dynamic_cast<Player*>(pPlayer);
        if (player)
        {
            AEGfxSetRenderMode(AE_GFX_RM_COLOR);
            AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.0f);
            AEGfxSetColorToAdd(1.0f, 0.87f, 0.00f, 0.25f);
            AEGfxSetBlendMode(AE_GFX_BM_BLEND);
            tf.SetPosition(player->transform.position.x, player->transform.position.y);
            tf.SetUniformScale(player->GetAoeRadius());
            tf.Apply();
            AEGfxMeshDraw(Meshes::pCircleMesh, AE_GFX_MDM_TRIANGLES);
            AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
            AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
        }
    }

    // 5. Entity glow halos
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.0f);
    for (auto& obj : gamePageObj)
    {
        if (!obj->isActive) continue;

        if (obj == pPlayer)
        {
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
            if (b->owner == BulletOwner::PLAYER) AEGfxSetColorToAdd(1.0f, 0.9f, 0.0f, 0.3f);
            else                                 AEGfxSetColorToAdd(1.0f, 0.1f, 0.0f, 0.3f);
            tf.SetPosition(obj->transform.position.x, obj->transform.position.y);
            tf.SetUniformScale(obj->transform.scale.x * 2.8f);
            tf.Apply();
            AEGfxMeshDraw(Meshes::pCircleMesh, AE_GFX_MDM_TRIANGLES);
        }
    }

    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

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

    // 7. Particles
    Player* player = dynamic_cast<Player*>(pPlayer);
    if (player)
    {
        player->smokePS.Render();
        for (GameObject* obj : gamePageObj)
        {
            if (obj->ObjectType != NP) continue;
            NPC* npc = dynamic_cast<NPC*>(obj);
            if (npc && npc->hasExploded) npc->explosionParticles.Render();
        }
    }

    // 8. World-space health bars
    gameUI.DrawAllHealthBars();

    // 9. Minimap
    DrawMinimap(gamePageObj, sCamX, sCamY);

    AEGfxSetCamPosition(sCamX, sCamY);

    // -----------------------------------------------------------------------
    // SCREEN SPACE
    // -----------------------------------------------------------------------
    AEGfxSetCamPosition(0.0f, 0.0f);

    ammoText.Draw();
    gameUI.DrawHealthText();
    gameUI.DrawXPBar();
    gameUI.DrawCurrentStats();
    gameUI.DrawWaveInfo();
    gameUI.DrawWaveTimer();
    gameUI.DrawAbilities();

    if (powerUpSystem.IsWaitingForUpgrade())
        gameUI.DrawPowerUpScreen();

    if (AEInputCheckTriggered(AEVK_U))
        powerUpSystem.AddExperience(200.0f);

    // Wave announcement banner
    if (s_WaveAnnounceTimer > 0.0f && !powerUpSystem.IsWaitingForUpgrade())
    {
        const f32 t = s_WaveAnnounceTimer / WAVE_ANNOUNCE_DURATION;
        f32 alpha = 0.0f;
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

    // Pause overlay
    if (isPaused)
    {
        PausePage_Draw();
        PausePage_Update();
    }

    AEGfxSetCamPosition(sCamX, sCamY);
}

// ============================================================================
// Game_Free
// ============================================================================
void Game_Free()
{
    waveSystem.Cleanup();
    PausePage_Free();

    for (auto& obj : gamePageObj)
    {
        delete obj;
        obj = nullptr;
    }
    gamePageObj.clear();
    gamePageObj.shrink_to_fit();
}

// ============================================================================
// Game_Unload
// ============================================================================
void Game_Unload()
{
    Meshes::FreeMeshes();
    NPC_UnloadTextures();
    AEGfxDestroyFont(gameFont);
    PausePage_Unload();
}