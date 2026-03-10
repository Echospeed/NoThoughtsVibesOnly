// ============================================================================
// WaveSystem.hpp - Wave and Round Management System
// ============================================================================
// Manages progressive enemy waves with:
// - Multiple rounds of increasing difficulty
// - Wave spawning with configurable enemy counts
// - Boss waves every N rounds
// - Experience rewards for clearing waves
// - Level-based wave limits (Level 1: 5 waves, Level 2: 10 waves, Endless: infinite)
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
// Defines the composition of a single enemy wave
struct WaveConfig
{
    u32 walkerCount{0};     // Number of walking enemies
    u32 meleeCount{0};      // Number of melee enemies  
    u32 rangerCount{0};     // Number of ranged enemies
    bool hasBoss{false};    // Whether this wave includes a boss
    f32 expReward{50.0f};   // Experience rewarded for clearing wave
};

// ============================================================================
// WAVE SYSTEM CLASS
// ============================================================================
// Central controller for wave-based gameplay
class WaveSystem
{
public:
    // === Lifecycle Methods ===
    
    // Initializes the wave system
    void Init(Player* player);
    
    // Updates wave state (checks for wave completion)
    void Update(f32 deltaTime);
    
    // Cleans up wave system
    void Cleanup();
    
    // === Level Config ===
    
    // Sets the level configuration (call after Init)
    void SetLevelConfig(const LevelConfig& config) { levelConfig = config; }
    
    // Gets the current level configuration
    const LevelConfig& GetLevelConfig() const { return levelConfig; }
    
    // Checks if all waves are complete (for non-endless levels)
    bool IsLevelComplete() const;
    
    // Gets max waves for current level (UINT32_MAX = endless)
    u32 GetMaxWaves() const { return levelConfig.numWaves; }
    
    // === Wave Control ===
    
    // Starts the next wave
    void StartNextWave();
    
    // Checks if current wave is complete (all enemies dead)
    bool IsWaveComplete() const;
    
    // Gets current wave number
    u32 GetCurrentWave() const { return currentWave; }
    
    // Gets current round number
    u32 GetCurrentRound() const { return currentRound; }
    
    // Checks if in between waves
    bool IsInBreak() const { return inWaveBreak; }
    
    // Gets time until next wave
    f32 GetBreakTimeRemaining() const { return waveBreakTimer; }
    
private:
    // === Internal State ===
    Player* playerRef{nullptr};         // Reference to player for exp rewards
    u32 currentWave{0};                 // Current wave number (0 = not started)
    u32 currentRound{1};                // Current round number
    bool inWaveBreak{false};            // Whether between waves
    f32 waveBreakTimer{0.0f};           // Time remaining in break
    const f32 WAVE_BREAK_DURATION{2.0f}; // Seconds between waves
    LevelConfig levelConfig{};          // Current level configuration
    
    // === Wave Generation ===
    
    // Generates wave configuration based on current round
    WaveConfig GenerateWaveConfig() const;
    
    // Spawns enemies for the given wave configuration
    void SpawnWave(const WaveConfig& config);
    
    // Spawns a boss enemy
    void SpawnBoss();
    
    // === Difficulty Scaling ===
    
    // Calculates enemy count for current round
    u32 GetEnemyCountForRound() const;
    
    // Checks if current round should have a boss
    bool ShouldSpawnBoss() const;
};
