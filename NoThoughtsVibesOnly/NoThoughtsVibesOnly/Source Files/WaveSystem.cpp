// ============================================================================
// WaveSystem.cpp - Enemy Wave Scheduling and Spawning
// ============================================================================
// Manages progressive waves of enemies across multiple rounds.
//
// LEVEL MODES:
// ----------------------------------------------------------------------------
//   Level 1  : 5 waves, no boss
//   Level 2  : 10 waves, boss on final wave
//   Endless  : Infinite waves, boss every 5 rounds
//
// WAVE FLOW:
// ----------------------------------------------------------------------------
//   1. Player presses C -> StartNextWave() -> SpawnWave(config)
//   2. Enemies fight the player.
//   3. WaveSystem::Update() detects IsWaveComplete() (all NPC objects inactive).
//   4. A WAVE_BREAK_DURATION second countdown begins (shown in HUD).
//   5. After the countdown, StartNextWave() fires automatically.
//   6. Boss spawns based on level config rules.
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
    waveStartTime = 0.0f;
    levelConfig = ::GetLevelConfig(LevelType::ENDLESS); // Default, will be overwritten by SetLevelConfig
}

// ============================================================================
// IsLevelComplete
// ============================================================================
// Returns true if all waves for this level are cleared (non-endless only)
// ============================================================================
bool WaveSystem::IsLevelComplete() const
{
    // Endless mode never completes via wave count
    if (levelConfig.type == LevelType::ENDLESS)
        return false;

    // Level complete when we've cleared the final wave and no enemies remain
    return currentWave >= levelConfig.numWaves && IsWaveComplete() && !inWaveBreak;
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
        {
            // Check if we've reached max waves for non-endless levels
            if (levelConfig.type != LevelType::ENDLESS && currentWave >= levelConfig.numWaves)
            {
                // Don't start another wave - level is complete
                inWaveBreak = false;
                return;
            }
            StartNextWave();
        }
        return; // Don't check for wave completion during a break
    }

    if (currentWave > 0 && IsWaveComplete())
    {
        // Show different message for final wave vs continuing
        if (levelConfig.type != LevelType::ENDLESS && currentWave >= levelConfig.numWaves)
        {
            // Don't set inWaveBreak - let IsLevelComplete() return true
        }
        else
        {
            AccumulateWaveScore(waveTookDamage);
            ++currentRound;
            inWaveBreak = true;
            waveBreakTimer = WAVE_BREAK_DURATION;
        }
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
    waveStartTime = static_cast<float>(AEGetTime(nullptr));

    const WaveConfig config = GenerateWaveConfig();
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
    const auto& wc = GameConfig::Wave();
    const u32   base = GetEnemyCountForRound();

    if (currentRound <= static_cast<unsigned int>(wc.earlyRoundThreshold))
    {
        config.walkerCount = base;
        config.meleeCount = currentRound;
        config.rangerCount = 0;
    }
    else if (currentRound <= static_cast<unsigned int>(wc.midRoundThreshold))
    {
        config.walkerCount = base / wc.midWalkerDivisor;
        config.meleeCount = base / wc.midMeleeDivisor;
        config.rangerCount = base / wc.midRangerDivisor;
    }
    else
    {
        config.walkerCount = base / wc.lateWalkerDivisor;
        config.meleeCount = base / wc.lateMeleeDivisor;
        config.rangerCount = base / wc.lateRangerDivisor;
    }

    config.hasBoss = ShouldSpawnBoss();
    config.expReward = wc.expRewardBase + (currentRound * wc.expRewardPerRound);
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

    // --- Rangers (each gets 3 freshly allocated bullets on spawn) ---
    // Bullets are owned by the Ranger and freed back to the object list
    // when the Ranger dies (NPC::Update clears startPos on death).
    for (u32 i = 0; i < config.rangerCount; ++i)
    {
        NPC* ranger = SpawnNPC(NPC_RANGER);

        for (int b = 0; b < GameConfig::Npc().ranger.bulletsAssigned; ++b)
        {
            Bullet* bullet = new Bullet();
            bullet->owner = BulletOwner::ENEMY;
            bullet->startPos = ranger;
            bullet->isActive = false;
            bullet->spent = false;
            bullet->spriteRenderer.colour = { 1.0f, 0.0f, 0.0f, 0.0f }; // Hidden
            bullet->Start();
        }

    }

    // --- Boss ---
    if (config.hasBoss) SpawnBoss();

    //const u32 totalSpawned = config.walkerCount + config.meleeCount + config.rangerCount;
}

// ============================================================================
// SpawnBoss
// ============================================================================
// Spawns one Boss NPC and allocates 8 fresh bullets directly to it.
// No shared pool needed - bullets are created on demand and freed when
// the Boss dies (NPC::Update clears startPos on death, Game_Free deletes all).
// ============================================================================
void WaveSystem::SpawnBoss()
{
    if (!playerRef) return;

    NPC* boss = new NPC();
    boss->ObjectType = NP;
    boss->type = NPC_BOSS;
    boss->target = playerRef;
    boss->Start();

    for (int b = 0; b < GameConfig::Npc().boss.bulletsAssigned; ++b)
    {
        Bullet* bullet = new Bullet();
        bullet->owner = BulletOwner::ENEMY;
        bullet->startPos = boss;
        bullet->isActive = false;
        bullet->spent = false;
        bullet->spriteRenderer.colour = { 1.0f, 0.0f, 1.0f, 0.0f }; // Hidden (magenta when fired)
        bullet->Start();
    }
}

// ============================================================================
// GetEnemyCountForRound
// ============================================================================
// Returns the base enemy count, scaling with round and capped at 20.
// ============================================================================
u32 WaveSystem::GetEnemyCountForRound() const
{
    const auto& wc = GameConfig::Wave();
    const u32 count = static_cast<unsigned int>(wc.baseEnemyCount) + (currentRound * static_cast<unsigned int>(wc.enemyCountPerRound));
    return count > static_cast<unsigned int>(wc.maxEnemyCount) ? static_cast<unsigned int>(wc.maxEnemyCount) : count;
}

// ============================================================================
// ShouldSpawnBoss
// ============================================================================
// Returns true based on level configuration:
//   - Level 1: No boss (hasBoss = false)
//   - Level 2: Boss on final wave (wave 10)
//   - Endless: Boss every 5 rounds
// ============================================================================
bool WaveSystem::ShouldSpawnBoss() const
{
    // If level config says no boss, never spawn one
    if (!levelConfig.hasBoss)
        return false;

    // For non-endless levels with boss enabled, spawn boss on final wave
    if (levelConfig.type != LevelType::ENDLESS)
        return currentWave >= levelConfig.numWaves;

    // Endless mode: boss every 5 rounds
    return (currentRound % 5) == 0;
}

// ============================================================================
// AccumulateWaveScore
// ============================================================================
// Called when a wave clears. Applies speed multiplier and no-damage bonus,
// then adds to the running total exposed via GetAccumulatedScore().
// ============================================================================
void WaveSystem::AccumulateWaveScore(bool tookDamage)
{
    const auto& cfg = GameConfig::Gameplay();

    const int base = (levelConfig.type == LevelType::ENDLESS)
        ? cfg.scorePerWaveEndless
        : cfg.scorePerWaveNormal;

    // Speed multiplier: speedBonusMax at instant clear, 1.0x at par time, clamped
    const float elapsed = static_cast<float>(AEGetTime(nullptr)) - waveStartTime;
    const float par = cfg.waveParTimeSeconds;
    const float ratio = (par > 0.0f) ? AEClamp(elapsed / par, 0.0f, 1.0f) : 1.0f;
    const float speedMul = cfg.speedBonusMax + (1.0f - cfg.speedBonusMax) * ratio;

    int waveScore = static_cast<int>(static_cast<float>(base) * speedMul);

    // No-damage bonus (flat, after speed multiply)
    if (!tookDamage)
        waveScore += cfg.noDamageBonus;

    accumulatedScore += waveScore;
    waveTookDamage = false;  // Reset for next wave
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
    waveStartTime = 0.0f;
    accumulatedScore = 0;
    waveTookDamage = false;
    waveTookDamage = false;
}