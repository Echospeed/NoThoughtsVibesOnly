// Author: John Chiow
// Co-Author: Liu Yan Bin, Stanley Lu

// ============================================================================
// PowerUpSystem.hpp - Player Power-Up and Progression System
// ============================================================================
// Manages:
// - Player experience and leveling
// - Power-up choices on level up
// - Stat upgrades (Speed, Bullet Damage, AoE)
// - Enemy scaling with rounds
// ============================================================================

#pragma once
#include "pch.hpp"

// Forward declarations
class Player;
class NPC;

// ============================================================================
// POWER-UP TYPES
// ============================================================================
enum PowerUpType
{
    POWERUP_SPEED,          // Increase movement speed
    POWERUP_BULLET_DAMAGE,  // Increase bullet damage
    POWERUP_AOE_DAMAGE,     // Increase AoE radius and damage
    POWERUP_LIFESTEAL       // Increase heal-on-kill amount
};

// ============================================================================
// POWER-UP DATA
// ============================================================================
struct PowerUp
{
    PowerUpType type;
    const char* name;
    const char* description;
    f32 value;              // How much the stat increases
};

// ============================================================================
// PLAYER STATS (Upgradeable)
// ============================================================================
struct PlayerStats
{
    // === Movement ===
    f32 baseSpeed{ 200.0f };
    f32 speedBonus{ 0.0f };           // Flat bonus from upgrades
    u32 speedUpgrades{ 0 };           // Times upgraded

    // === Combat ===
    f32 baseBulletDamage{ 100.0f };
    f32 bulletDamageBonus{ 0.0f };    // Flat bonus from upgrades
    u32 bulletDamageUpgrades{ 0 };    // Times upgraded
    u32 bulletCount{ 10 };            // Current pool size (starts at 10, +5 per upgrade)

    // === AoE ===
    f32 baseAoeRadius{ 100.0f };      // Base AoE radius
    f32 aoeRadiusBonus{ 0.0f };       // Bonus radius
    f32 baseAoeDamage{ 100.0f };      // Base AoE damage per second
    f32 aoeDamageBonus{ 0.0f };       // Bonus damage
    u32 aoeUpgrades{ 0 };             // Times upgraded

    // === Experience ===
    f32 currentExp{ 0.0f };
    f32 expToNextLevel{ 50.0f };
    u32 level{ 1 };

    // === Lifesteal ===
    f32 lifestealBonus{ 0.0f };   // Extra HP healed per kill (on top of base 5)

    // === Methods ===
    f32 GetTotalSpeed() const { return baseSpeed + speedBonus; }
    f32 GetTotalBulletDamage() const { return baseBulletDamage + bulletDamageBonus; }
    f32 GetTotalAoeRadius() const { return baseAoeRadius + aoeRadiusBonus; }
    f32 GetTotalAoeDamage() const { return baseAoeDamage + aoeDamageBonus; }
};

// ============================================================================
// POWER-UP SYSTEM CLASS
// ============================================================================
class PowerUpSystem
{
public:
    // === Initialization ===
    void Init();

    // === Experience Management ===
    void AddExperience(f32 amount);
    bool CheckLevelUp();                // Returns true if leveled up

    // === Power-Up Selection ===
    void GeneratePowerUpChoices();      // Creates 3 random choices
    void ApplyPowerUp(PowerUpType type, Player* player);

    // === Enemy Scaling ===
    f32 GetRangerDamageForRound(u32 round) const;
    f32 GetMeleeSpeedForRound(u32 round) const;
    f32 GetMeleeDamageForRound(u32 round) const;

    // === Getters ===
    PlayerStats& GetStats() { return stats; }
    const PlayerStats& GetStats() const { return stats; }
    const PowerUp* GetPowerUpChoices() const { return powerUpChoices; }
    bool IsWaitingForUpgrade() const { return waitingForUpgrade; }
    void SetWaitingForUpgrade(bool waiting) { waitingForUpgrade = waiting; }

private:
    PlayerStats stats;
    PowerUp powerUpChoices[4]{};          // 4 choices on level up
    bool waitingForUpgrade{ false };      // Pauses game for upgrade choice

    // Helper methods
    PowerUp CreatePowerUp(PowerUpType type);
};