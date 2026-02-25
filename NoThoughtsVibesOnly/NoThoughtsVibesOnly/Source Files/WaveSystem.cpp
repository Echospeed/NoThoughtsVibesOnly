// ============================================================================
// WaveSystem.cpp - Enemy Wave Scheduling and Spawning
// ============================================================================
// Manages progressive waves of enemies across multiple rounds.
//
// WAVE FLOW:
// ----------------------------------------------------------------------------
//   1. Player presses C -> StartNextWave() -> SpawnWave(config)
//   2. Enemies fight the player.
//   3. WaveSystem::Update() detects IsWaveComplete() (all NPC objects inactive).
//   4. A WAVE_BREAK_DURATION second countdown begins (shown in HUD).
//   5. After the countdown, StartNextWave() fires automatically.
//   6. Every 5 rounds a Boss is added to the wave.
//
// ENEMY COUNT SCALING:
// ----------------------------------------------------------------------------
//   Base enemies = 5 + (round * 2), capped at 20.
//   Rounds 1-3   : Mostly walkers + a few melee.
//   Rounds 4-7   : Mixed walkers, melee, rangers.
//   Rounds 8+    : Heavy ranger/melee mix.
//
// RANGER BULLET ASSIGNMENT:
// ----------------------------------------------------------------------------
//   Each Ranger NPC is assigned 3 bullet slots from the global enemy pool.
//   Boss NPCs are assigned 8 bullet slots.
//   Bullets with startPos == nullptr are unassigned and available.
// ============================================================================

#include "pch.hpp"
#include "WaveSystem.hpp"
#include "GamePage.hpp"
#include "Player.hpp"
#include "NPC.hpp"
#include "Bullet.hpp"
#include <iostream>

// ============================================================================
// Init
// ============================================================================
void WaveSystem::Init(Player* player)
{
    playerRef = player;
    currentWave = 0;
    currentRound = 1;
    inWaveBreak = false;
    waveBreakTimer = 0.0f;

    std::cout << "\n==============================================\n"
        << "    WAVE SYSTEM READY - Press C to begin!\n"
        << "==============================================\n\n";
}

// ============================================================================
// Update
// ============================================================================
// Handles the inter-wave countdown. When the timer expires, the next wave
// starts automatically. Also detects wave completion while a wave is running.
// ============================================================================
void WaveSystem::Update(f32 deltaTime)
{
    if (inWaveBreak)
    {
        waveBreakTimer -= deltaTime;
        if (waveBreakTimer <= 0.0f)
            StartNextWave();

        return; // Don't check for wave completion during a break
    }

    if (currentWave > 0 && IsWaveComplete())
    {
        std::cout << "\n==============================================\n"
            << "    WAVE " << currentWave << " CLEARED!\n"
            << "    Next wave in " << WAVE_BREAK_DURATION << "s...\n"
            << "==============================================\n\n";

        ++currentRound;
        inWaveBreak = true;
        waveBreakTimer = WAVE_BREAK_DURATION;
    }
}

// ============================================================================
// StartNextWave
// ============================================================================
// Increments wave counter, generates a config, and spawns the enemies.
// ============================================================================
void WaveSystem::StartNextWave()
{
    ++currentWave;
    inWaveBreak = false;

    const WaveConfig config = GenerateWaveConfig();

    std::cout << "\n==============================================\n"
        << "    WAVE " << currentWave << "  (Round " << currentRound << ")\n"
        << "    Walkers: " << config.walkerCount
        << "  Melee: " << config.meleeCount
        << "  Rangers: " << config.rangerCount
        << (config.hasBoss ? "  [BOSS WAVE!]" : "")
        << "\n==============================================\n\n";

    SpawnWave(config);
}

// ============================================================================
// IsWaveComplete
// ============================================================================
// Returns true when no NPC GameObjects remain active in the scene.
// ============================================================================
bool WaveSystem::IsWaveComplete() const
{
    for (const GameObject* obj : gamePageObj)
    {
        if (obj && obj->isActive && obj->ObjectType == NP)
            return false;
    }
    return true;
}

// ============================================================================
// GenerateWaveConfig
// ============================================================================
// Scales enemy composition with the current round number.
// ============================================================================
WaveConfig WaveSystem::GenerateWaveConfig() const
{
    WaveConfig config{};
    const u32  base = GetEnemyCountForRound();

    if (currentRound <= 3)
    {
        // Early rounds: walkers only, with a few melee
        config.walkerCount = base;
        config.meleeCount = currentRound; // 1, 2, or 3
        config.rangerCount = 0;
    }
    else if (currentRound <= 7)
    {
        // Mid rounds: mixed composition
        config.walkerCount = base / 2;
        config.meleeCount = base / 3;
        config.rangerCount = base / 4;
    }
    else
    {
        // Late rounds: rangers dominate
        config.walkerCount = base / 3;
        config.meleeCount = base / 3;
        config.rangerCount = base / 2;
    }

    config.hasBoss = ShouldSpawnBoss();
    config.expReward = 50.0f + (currentRound * 25.0f);
    return config;
}

// ============================================================================
// SpawnWave
// ============================================================================
// Creates NPC instances and assigns bullet pool slots to Ranger/Boss NPCs.
// ============================================================================
void WaveSystem::SpawnWave(const WaveConfig& config)
{
    if (!playerRef) return;

    // Helper lambda to create an NPC of a given type
    auto SpawnNPC = [&](NPCType npcType)
        {
            NPC* n = new NPC();
            n->ObjectType = NP;
            n->type = npcType;
            n->target = playerRef;
            n->Start();
            return n;
        };

    // --- Walkers ---
    for (u32 i = 0; i < config.walkerCount; ++i)
        SpawnNPC(NPC_WALK);

    // --- Melee ---
    for (u32 i = 0; i < config.meleeCount; ++i)
        SpawnNPC(NPC_MELEE);

    // --- Rangers (each needs 3 bullet pool slots) ---
    for (u32 i = 0; i < config.rangerCount; ++i)
    {
        NPC* ranger = SpawnNPC(NPC_RANGER);

        u32 bulletsAssigned = 0;
        int totalBullets = 0;
        int enemyBullets = 0;
        int availableEnemyBullets = 0;

        for (auto& obj : gamePageObj)
        {
            if (obj->ObjectType != SHOT) continue;
            ++totalBullets;

            Bullet* b = dynamic_cast<Bullet*>(obj);
            if (!b || b->owner != BulletOwner::ENEMY) continue;
            ++enemyBullets;

            if (b->startPos != nullptr) continue;
            ++availableEnemyBullets;

            // Assign this bullet slot to the new Ranger
            b->startPos = ranger;
            b->isActive = false;
            ++bulletsAssigned;

            if (bulletsAssigned >= 3) break; // Each Ranger gets 3 bullets
        }

        std::cout << "[WaveSystem] Ranger #" << (i + 1)
            << " assigned " << bulletsAssigned << " bullets"
            << " (pool: " << availableEnemyBullets << " free)\n";
    }

    // --- Boss ---
    if (config.hasBoss) SpawnBoss();

    const u32 totalSpawned = config.walkerCount + config.meleeCount + config.rangerCount;
    std::cout << "[WaveSystem] Spawned " << totalSpawned << " enemies"
        << (config.hasBoss ? " + BOSS" : "") << "\n";
}

// ============================================================================
// SpawnBoss
// ============================================================================
// Spawns one Boss NPC and assigns 8 bullet pool slots to it.
// ============================================================================
void WaveSystem::SpawnBoss()
{
    if (!playerRef) return;

    std::cout << "\n==============================================\n"
        << "    *** BOSS INCOMING! ***\n"
        << "==============================================\n\n";

    NPC* boss = new NPC();
    boss->ObjectType = NP;
    boss->type = NPC_BOSS;
    boss->target = playerRef;
    boss->Start();

    u32 bulletsAssigned = 0;
    for (auto& obj : gamePageObj)
    {
        if (obj->ObjectType != SHOT) continue;

        Bullet* b = dynamic_cast<Bullet*>(obj);
        if (!b || b->owner != BulletOwner::ENEMY || b->startPos != nullptr) continue;

        b->startPos = boss;
        ++bulletsAssigned;
        if (bulletsAssigned >= 8) break; // Boss fires 8-way volley
    }

    std::cout << "[WaveSystem] Boss spawned with " << bulletsAssigned << " bullet slots.\n";
}

// ============================================================================
// GetEnemyCountForRound
// ============================================================================
// Returns the base enemy count, scaling with round and capped at 20.
// ============================================================================
u32 WaveSystem::GetEnemyCountForRound() const
{
    const u32 count = 5 + (currentRound * 2);
    return count > 20 ? 20 : count;
}

// ============================================================================
// ShouldSpawnBoss
// ============================================================================
// Returns true every 5 rounds (rounds 5, 10, 15, ...).
// ============================================================================
bool WaveSystem::ShouldSpawnBoss() const
{
    return (currentRound % 5) == 0;
}

// ============================================================================
// Cleanup
// ============================================================================
// Resets all wave state. Call in Game_Free() before deleting game objects.
// ============================================================================
void WaveSystem::Cleanup()
{
    playerRef = nullptr;
    currentWave = 0;
    currentRound = 1;
    inWaveBreak = false;
    waveBreakTimer = 0.0f;
}