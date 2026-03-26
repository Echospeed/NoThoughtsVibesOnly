#pragma once
// ============================================================================
// GameConfig.hpp - Centralised Game Configuration (RapidJSON-backed)
// ============================================================================
// All previously-hardcoded gameplay values are loaded from JSON files.
//
// JSON FILES (place all in Assets/):
// ----------------------------------------------------------------------------
//   player_config.json   -> PlayerConfig
//   npc_config.json      -> NpcConfig  (all 4 NPC types)
//   powerup_config.json  -> PowerUpConfig
//   wave_config.json     -> WaveSpawnConfig
//   gameplay_config.json -> GameplayConfig (world, minimap, bullet)
//
// USAGE:
// ----------------------------------------------------------------------------
//   // Call once in Game_Load():
//   GameConfig::Load();
//
//   // Read anywhere:
//   f32 spd = GameConfig::Player().baseSpeed;
//   f32 hp  = GameConfig::Npc().boss.health;
//
// NOTE: Implementation lives in GameConfig.cpp (required for C++14).
// ============================================================================

#include "pch.hpp"
#include "include/rapidjson/document.h"
#include "include/rapidjson/istreamwrapper.h"
#include <fstream>
#include <iostream>
#include <string>

// ============================================================================
// PlayerConfig
// ============================================================================
struct PlayerConfig
{
    f32 startHealth{ 100.0f };
    f32 maxHealth{ 100.0f };
    f32 baseSpeed{ 200.0f };
    f32 rotationSpeed{ 3.0f };
    f32 scaleX{ 50.0f };
    f32 scaleY{ 50.0f };
    f32 shootCooldown{ 0.10f };
    f32 shootSuppressCooldown{ 0.30f };
    f32 reloadDuration{ 2.0f };
    int   startingAmmo{ 20 };
    f32 bulletLifeTime{ 4.0f };
    f32 bulletSpeed{ 1500.0f };
    f32 spawnClearRadius{ 100.0f };
    std::string texture{ "Assets/shipYellow_manned.png" };
};

// ============================================================================
// NpcTypeConfig  (one per NPC type)
// ============================================================================
struct NpcTypeConfig
{
    f32 health{ 100.0f };
    f32 speed{ 200.0f };
    f32 scaleX{ 50.0f };
    f32 scaleY{ 50.0f };
    f32 fireRate{ 1.0f };
    f32 retreatDistance{ 250.0f };
    f32 velocityDamping{ 0.95f };
    f32 changeDirInterval{ 2.0f };
    f32 orbitDistance{ 300.0f };
    f32 orbitMargin{ 50.0f };
    f32 explosionDamage{ 40.0f };
    int   explosionBurst{ 20 };
    int   bulletVolleyCount{ 8 };
    int   bulletsAssigned{ 6 };
    int   burstCount{ 20 };
    f32 xpReward{ 25.0f };
    f32 baseHeal{ 5.0f };
    f32 colourR{ 1.0f };
    f32 colourG{ 1.0f };
    f32 colourB{ 1.0f };
    std::string texture{};
};

struct NpcConfig
{
    NpcTypeConfig walk{};
    NpcTypeConfig melee{};
    NpcTypeConfig ranger{};
    NpcTypeConfig boss{};
};

// ============================================================================
// PowerUpConfig
// ============================================================================
struct EnemyScalingConfig
{
    f32 rangerBaseDamage{ 25.0f };
    f32 rangerDamagePerRound{ 5.0f };
    f32 meleeBaseSpeed{ 250.0f };
    f32 meleeSpeedPerRound{ 10.0f };
    f32 meleeSpeedCap{ 400.0f };
    f32 meleeBaseDamage{ 50.0f };
    f32 meleeDamagePerRound{ 5.0f };
};

struct PowerUpConfig
{
    f32 baseSpeed{ 200.0f };
    f32 baseBulletDamage{ 100.0f };
    f32 baseAoeRadius{ 100.0f };
    f32 baseAoeDamage{ 100.0f };
    int   startingBulletCount{ 10 };
    f32 expToFirstLevel{ 100.0f };
    f32 expScalePerLevel{ 1.1f };
    f32 speedUpgradeBonus{ 200.0f };
    f32 bulletDamageBonus{ 25.0f };
    int   bulletCountBonus{ 5 };
    f32 aoeRadiusBonus{ 25.0f };
    f32 aoeDamageBonus{ 25.0f };
    f32 lifestealBonus{ 10.0f };
    f32 lifestealBaseHealPerKill{ 5.0f };
    EnemyScalingConfig enemyScaling{};
};

// ============================================================================
// WaveSpawnConfig
// ============================================================================
struct ContactDamageConfig
{
    f32 melee{ 50.0f };
    f32 boss{ 100.0f };
    f32 walkRanger{ 20.0f };
};

struct KnockbackConfig
{
    f32 melee{ 300.0f };
    f32 boss{ 500.0f };
    f32 walkRanger{ 200.0f };
};

struct WaveSpawnConfig
{
    f32 waveBreakDuration{ 2.0f };
    int   baseEnemyCount{ 5 };
    int   enemyCountPerRound{ 2 };
    int   maxEnemyCount{ 20 };
    int   earlyRoundThreshold{ 3 };
    int   midRoundThreshold{ 7 };
    int   midWalkerDivisor{ 2 };
    int   midMeleeDivisor{ 3 };
    int   midRangerDivisor{ 4 };
    int   lateWalkerDivisor{ 3 };
    int   lateMeleeDivisor{ 3 };
    int   lateRangerDivisor{ 2 };
    f32 expRewardBase{ 50.0f };
    f32 expRewardPerRound{ 25.0f };
    ContactDamageConfig contactDamage{};
    KnockbackConfig     knockback{};
};

// ============================================================================
// GameplayConfig  (world bounds, minimap, bullet, pools, scoring, abilities)
// ============================================================================
struct MinimapConfig
{
    f32 worldRefWidth{ 2500.0f };
    f32 size{ 200.0f };
    f32 screenOffsetX{ 650.0f };
    f32 screenOffsetY{ -300.0f };
    f32 playerDotSize{ 12.0f };
    f32 npcDotSize{ 5.0f };
};

struct WorldConfig
{
    f32 width{ 2500.0f };
    f32 height{ 2500.0f };
};

struct BulletConfig
{
    f32 playerSpeed{ 1500.0f };
    f32 enemySpeed{ 800.0f };
    f32 maxLifeTime{ 4.0f };
    f32 enemyDamage{ 25.0f };
    f32 defaultScale{ 10.0f };
};

struct GameplayConfig
{
    MinimapConfig minimap{};
    WorldConfig   world{};
    BulletConfig  bullet{};

    // --- NEW: previously hardcoded in GamePage.cpp ---
    int playerBulletPoolSize{ 10 };   // gameplay_config.json: "playerBulletPoolSize"
    int enemyBulletPoolSize{ 60 };    // gameplay_config.json: "enemyBulletPoolSize"

    // --- NEW: previously hardcoded in GamePage.cpp ---
    int scorePerWaveNormal{ 100 };    // gameplay_config.json: "scorePerWaveNormal"
    int scorePerWaveEndless{ 150 };   // gameplay_config.json: "scorePerWaveEndless"

    // --- NEW: previously hardcoded in Abilities.hpp ---
    f32 invulnDuration{ 3.0f };       // gameplay_config.json: "invulnDuration"
    f32 invulnCooldown{ 30.0f };      // gameplay_config.json: "invulnCooldown"
};

// ============================================================================
// GameConfig namespace  -  declarations only
// Implementation is in GameConfig.cpp
// ============================================================================
namespace GameConfig
{
    // Call once in Game_Load() before anything else
    void Load();

    // Accessors - safe to call before Load() (auto-loads on first access)
    const PlayerConfig& Player();
    const NpcConfig& Npc();
    const PowerUpConfig& PowerUp();
    const WaveSpawnConfig& Wave();
    const GameplayConfig& Gameplay();

} // namespace GameConfig