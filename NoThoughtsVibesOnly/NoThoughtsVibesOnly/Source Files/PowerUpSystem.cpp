// ============================================================================
// PowerUpSystem.cpp - Player Progression and Power-Up Implementation
// ============================================================================
// See PowerUpSystem.hpp for class declaration and stat structure.
//
// FLOW:
// ----------------------------------------------------------------------------
//   1. Enemy dies -> NPC::Update() calls powerUpSystem.AddExperience(25).
//   2. AddExperience() checks for level-up via CheckLevelUp().
//   3. On level-up: GeneratePowerUpChoices() creates 4 options.
//   4. waitingForUpgrade = true  -> Game_Update() blocks gameplay.
//   5. Player presses 1/2/3/4  -> ApplyPowerUp() applies the chosen stat.
//   6. waitingForUpgrade = false -> gameplay resumes.
//
// SCALING:
// ----------------------------------------------------------------------------
//   Speed        : +200 flat per upgrade
//   Bullet damage: +25  flat per upgrade
//   AoE radius   : +20  per upgrade
//   AoE damage   : +25/sec per upgrade
//   XP threshold : x1.3 per level-up
// ============================================================================

#include "pch.hpp"
#include "PowerUpSystem.hpp"
#include "Player.hpp"
#include "NPC.hpp"
#include "Bullet.hpp"
#include "GamePage.hpp"
#include <iostream>

// ============================================================================
// Init
// ============================================================================
// Resets all stats to defaults and clears any pending upgrade state.
// Call once in Game_Init() after the player is created.
// ============================================================================
void PowerUpSystem::Init()
{
    stats = PlayerStats{};  // Reset to default values
    waitingForUpgrade = false;

    std::cout << "[PowerUp] Initialised. Level 1, 0/"
        << stats.expToNextLevel << " XP needed.\n";
}

// ============================================================================
// AddExperience
// ============================================================================
// Awards XP and triggers a level-up check.
// If the player levels up, power-up choices are generated and gameplay
// is paused (waitingForUpgrade = true).
// ============================================================================
void PowerUpSystem::AddExperience(f32 amount)
{
    stats.currentExp += amount;

    std::cout << "[XP] +" << amount << " XP  ("
        << stats.currentExp << " / " << stats.expToNextLevel << ")\n";

    if (CheckLevelUp())
    {
        GeneratePowerUpChoices();
        waitingForUpgrade = true;
    }
}

// ============================================================================
// CheckLevelUp
// ============================================================================
// Returns true if the current XP meets or exceeds the threshold.
// On level-up: increments level, carries over excess XP, and scales threshold.
// ============================================================================
bool PowerUpSystem::CheckLevelUp()
{
    if (stats.currentExp < stats.expToNextLevel) return false;

    // Carry over surplus XP so it isn't wasted
    stats.currentExp -= stats.expToNextLevel;
    stats.expToNextLevel *= 1.3f;    // Each level requires 30% more XP
    ++stats.level;

    std::cout << "\n============================================\n"
        << "           LEVEL UP! -> Level " << stats.level << "\n"
        << "       Choose your power-up!\n"
        << "============================================\n\n";

    return true;
}

// ============================================================================
// GeneratePowerUpChoices
// ============================================================================
// Always offers all 4 upgrades in a fixed order: Speed, Bullet Damage, AoE, Lifesteal.
// (Can be randomised later by shuffling the array if desired.)
// ============================================================================
void PowerUpSystem::GeneratePowerUpChoices()
{
    powerUpChoices[0] = CreatePowerUp(POWERUP_SPEED);
    powerUpChoices[1] = CreatePowerUp(POWERUP_BULLET_DAMAGE);
    powerUpChoices[2] = CreatePowerUp(POWERUP_AOE_DAMAGE);
    powerUpChoices[3] = CreatePowerUp(POWERUP_LIFESTEAL); // Always available as the 4th option
}

// ============================================================================
// CreatePowerUp
// ============================================================================
// Builds a PowerUp descriptor for the given type.
// The 'value' field is informational - actual stat changes happen in ApplyPowerUp.
// ============================================================================
PowerUp PowerUpSystem::CreatePowerUp(PowerUpType type)
{
    PowerUp p{};
    p.type = type;

    switch (type)
    {
    case POWERUP_SPEED:
        p.name = "SPEED BOOST";
        p.description = "+200 Movement Speed";
        p.value = 200.0f;
        break;

    case POWERUP_BULLET_DAMAGE:
        p.name = "BULLET POWER";
        p.description = "+25 Bullet Damage  +5 Bullets";
        p.value = 25.0f;
        break;

    case POWERUP_AOE_DAMAGE:
        p.name = "AoE MASTERY";
        p.description = "+20 AoE Radius  +25 Damage/sec";
        p.value = 20.0f;
        break;

    case POWERUP_LIFESTEAL:
        p.name = "LIFESTEAL";
        p.description = "+10 HP healed per kill"; // Stacks with base 5 HP heal on kill
        p.value = 10.0f;
        break;
    }

    return p;
}

// ============================================================================
// ApplyPowerUp
// ============================================================================
// Applies the chosen upgrade to the player's stats.
// Resumes gameplay by setting waitingForUpgrade = false.
// ============================================================================
void PowerUpSystem::ApplyPowerUp(PowerUpType type, Player* player)
{
    if (!player) return;

    switch (type)
    {
    case POWERUP_SPEED:
        stats.speedBonus += 200.0f;
        ++stats.speedUpgrades;
        std::cout << "[Upgrade] Speed -> " << stats.GetTotalSpeed() << "\n";
        break;

    case POWERUP_BULLET_DAMAGE:
        stats.bulletDamageBonus += 25.0f;
        ++stats.bulletDamageUpgrades;
        stats.bulletCount += 5;
        // Spawn 5 new bullet pool slots immediately into the live object list.
        // Game_Free() will delete them in its normal loop - no double-delete risk.
        for (int i = 0; i < 5; ++i)
        {
            Bullet* b = new Bullet();
            b->startPos = player; // Player-owned bullet
            b->isActive = false;
            b->spent = false;  // ready to fire
            b->owner = BulletOwner::PLAYER;
            b->spriteRenderer.colour = { 1.0f, 1.0f, 0.0f, 0.0f }; // Hidden
            b->Start();
        }
        std::cout << "[Upgrade] Bullet damage -> " << stats.GetTotalBulletDamage()
            << "  Bullet count -> " << stats.bulletCount << "\n";
        break;

    case POWERUP_AOE_DAMAGE:
        stats.aoeRadiusBonus += 25.0f;
        stats.aoeDamageBonus += 25.0f;
        ++stats.aoeUpgrades;
        std::cout << "[Upgrade] AoE radius -> " << stats.GetTotalAoeRadius()
            << "  damage -> " << stats.GetTotalAoeDamage() << "/sec\n";
        break;

    case POWERUP_LIFESTEAL:
        // Adds +10 to lifestealBonus. Total heal per kill = 5 (base) + lifestealBonus.
        // Boss kills use 15 (base) + lifestealBonus instead. See NPC.cpp death block.
        stats.lifestealBonus += 10.0f;
        std::cout << "[Upgrade] Lifesteal -> +" << (5.0f + stats.lifestealBonus) << " HP per kill\n";
        break;
    }

    waitingForUpgrade = false; // Resume gameplay
}

// ============================================================================
// Enemy Scaling Helpers
// ============================================================================
// These are used by WaveSystem / NPC logic to scale enemy stats per round.

f32 PowerUpSystem::GetRangerDamageForRound(u32 round) const
{
    return 25.0f + (round * 5.0f); // +5 damage per round
}

f32 PowerUpSystem::GetMeleeSpeedForRound(u32 round) const
{
    const f32 speed = 250.0f + (round * 10.0f);
    return speed > 400.0f ? 400.0f : speed; // Capped at 400
}

f32 PowerUpSystem::GetMeleeDamageForRound(u32 round) const
{
    return 50.0f + (round * 5.0f); // +5 damage per round
}