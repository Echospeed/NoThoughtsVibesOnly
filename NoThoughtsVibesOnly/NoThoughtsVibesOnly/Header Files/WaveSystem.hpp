// ============================================================================
// WaveSystem.hpp - Wave and Round Management System
// ============================================================================
// Manages progressive enemy waves with:
// - Multiple rounds of increasing difficulty
// - Wave spawning with configurable enemy counts
// - Boss waves every N rounds
// - Experience rewards for clearing waves
// - Level-based wave limits (Level 1: 5 waves, Level 2: 10 waves, Endless: infinite)
// - Per-wave skill scoring: speed multiplier + no-damage bonus
// ============================================================================
#pragma once
#include "pch.hpp"
#include "LevelConfig.hpp"
#include <vector>

// Forward declarations
class GameObject;
class Player;
class NPC;

// ============================================================================
// WAVE CONFIGURATION
// ============================================================================
struct WaveConfig
{
    u32 walkerCount{ 0 };
    u32 meleeCount{ 0 };
    u32 rangerCount{ 0 };
    bool hasBoss{ false };
    f32 expReward{ 50.0f };
};

// ============================================================================
// WAVE SYSTEM CLASS
// ============================================================================
class WaveSystem
{
public:
    // === Lifecycle Methods ===
    void Init(Player* player);
    void Update(f32 deltaTime);
    void Cleanup();

    // === Level Config ===
    void SetLevelConfig(const LevelConfig& config) { levelConfig = config; }
    const LevelConfig& GetLevelConfig() const { return levelConfig; }
    bool IsLevelComplete() const;
    u32  GetMaxWaves() const { return levelConfig.numWaves; }

    // === Wave Control ===
    void StartNextWave();
    bool IsWaveComplete() const;
    u32  GetCurrentWave()  const { return currentWave; }
    u32  GetCurrentRound() const { return currentRound; }
    bool IsInBreak()       const { return inWaveBreak; }
    f32  GetBreakTimeRemaining() const { return waveBreakTimer; }

    // === Scoring ===
    // Running total of wave scores (base × speed multiplier + no-damage bonuses).
    // GamePage reads this at game-end and adds the one-time boss kill bonus.
    int  GetAccumulatedScore() const { return accumulatedScore; }

    // GamePage calls this whenever the player takes damage, so WaveSystem
    // can track whether the wave was cleared without taking a hit.
    void NotifyPlayerDamaged() { waveTookDamage = true; }

    // NPC death block calls this to add per-kill score immediately
    void AddKillScore(int points) { accumulatedScore += points; }

private:
    // === Internal State ===
    Player* playerRef{ nullptr };
    u32  currentWave{ 0 };
    u32  currentRound{ 1 };
    bool inWaveBreak{ false };
    f32  waveBreakTimer{ 0.0f };
    const f32 WAVE_BREAK_DURATION{ 2.0f };
    LevelConfig levelConfig{};

    // === Scoring State ===
    float waveStartTime{ 0.0f };    // AEGetTime() snapshot at wave start
    int   accumulatedScore{ 0 };    // Running total across all cleared waves
    bool  waveTookDamage{ false };  // Set by NotifyPlayerDamaged(), reset each wave

    // Calculates and accumulates score for the wave that just cleared
    void AccumulateWaveScore(bool tookDamage);

    // === Wave Generation ===
    WaveConfig GenerateWaveConfig() const;
    void SpawnWave(const WaveConfig& config);
    void SpawnBoss();

    // === Difficulty Scaling ===
    u32  GetEnemyCountForRound() const;
    bool ShouldSpawnBoss() const;
};