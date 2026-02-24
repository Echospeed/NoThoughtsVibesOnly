// ============================================================================
// PowerUpSystem.cpp - Power-Up System Implementation
// ============================================================================
// Handles experience, leveling, and stat upgrades
// ============================================================================

#include "pch.hpp"
#include "PowerUpSystem.hpp"
#include "Player.hpp"
#include "NPC.hpp"
#include <iostream>
#include <cstdlib>

// ============================================================================
// Init - Initialize Power-Up System
// ============================================================================
void PowerUpSystem::Init()
{
    stats = PlayerStats{}; // Reset to defaults
    waitingForUpgrade = false;
    
    std::cout << "[POWERUP] System initialized. Level 1, 0/" 
              << stats.expToNextLevel << " XP\n";
}

// ============================================================================
// AddExperience - Award XP to Player
// ============================================================================
void PowerUpSystem::AddExperience(f32 amount)
{
    stats.currentExp += amount;
    
    std::cout << "[XP] Gained " << amount << " exp! (" 
              << stats.currentExp << "/" << stats.expToNextLevel << ")\n";
    
    // Check for level up
    if (CheckLevelUp())
    {
        GeneratePowerUpChoices();
        waitingForUpgrade = true; // Pause game for upgrade
    }
}

// ============================================================================
// CheckLevelUp - Check if Player Should Level Up
// ============================================================================
bool PowerUpSystem::CheckLevelUp()
{
    if (stats.currentExp >= stats.expToNextLevel)
    {
        // Level up!
        stats.level++;
        stats.currentExp -= stats.expToNextLevel;   // Carry over excess
        stats.expToNextLevel *= 1.3f;                // Scale requirement
        
        std::cout << "\n========================================\n";
        std::cout << "       ⭐ LEVEL UP! ⭐\n";
        std::cout << "       Level " << stats.level << "\n";
        std::cout << "       Choose your power-up!\n";
        std::cout << "========================================\n\n";
        
        return true;
    }
    
    return false;
}

// ============================================================================
// GeneratePowerUpChoices - Create 3 Random Upgrade Options
// ============================================================================
void PowerUpSystem::GeneratePowerUpChoices()
{
    // Always offer all 3 core options
    powerUpChoices[0] = CreatePowerUp(POWERUP_SPEED);
    powerUpChoices[1] = CreatePowerUp(POWERUP_BULLET_DAMAGE);
    powerUpChoices[2] = CreatePowerUp(POWERUP_AOE_DAMAGE);
}

// ============================================================================
// CreatePowerUp - Generate Power-Up Data
// ============================================================================
PowerUp PowerUpSystem::CreatePowerUp(PowerUpType type)
{
    PowerUp powerUp{};
    powerUp.type = type;
    
    switch (type)
    {
        case POWERUP_SPEED:
            powerUp.name = "SPEED BOOST";
            powerUp.description = "+200 Movement Speed";
            powerUp.value = 200.0f;
            break;
            
        case POWERUP_BULLET_DAMAGE:
            powerUp.name = "BULLET POWER";
            powerUp.description = "+25 Bullet Damage";
            powerUp.value = 25.0f;
            break;
            
        case POWERUP_AOE_DAMAGE:
            powerUp.name = "AoE MASTERY";
            powerUp.description = "+20 AoE Radius +25 Damage/sec";
            powerUp.value = 20.0f; // Radius bonus
            break;
    }
    
    return powerUp;
}

// ============================================================================
// ApplyPowerUp - Apply Chosen Upgrade to Player
// ============================================================================
void PowerUpSystem::ApplyPowerUp(PowerUpType type, Player* player)
{
    if (!player) return;
    
    switch (type)
    {
        case POWERUP_SPEED:
            stats.speedBonus += 200.0f;
            stats.speedUpgrades++;
            std::cout << "[UPGRADE] Speed increased! Total: " 
                      << stats.GetTotalSpeed() << "\n";
            break;
            
        case POWERUP_BULLET_DAMAGE:
            stats.bulletDamageBonus += 25.0f;
            stats.bulletDamageUpgrades++;
            std::cout << "[UPGRADE] Bullet damage increased! Total: " 
                      << stats.GetTotalBulletDamage() << "\n";
            break;
            
        case POWERUP_AOE_DAMAGE:
            stats.aoeRadiusBonus += 20.0f;
            stats.aoeDamageBonus += 25.0f;
            stats.aoeUpgrades++;
            std::cout << "[UPGRADE] AoE upgraded! Radius: " 
                      << stats.GetTotalAoeRadius() 
                      << ", Damage: " << stats.GetTotalAoeDamage() << "/sec\n";
            break;
    }
    
    waitingForUpgrade = false; // Resume game
}

// ============================================================================
// Enemy Scaling Functions
// ============================================================================

// Ranger bullet damage scales with round number
f32 PowerUpSystem::GetRangerDamageForRound(u32 round) const
{
    f32 baseDamage = 25.0f;
    f32 damagePerRound = 5.0f;  // +5 damage per round
    
    return baseDamage + (round * damagePerRound);
}

// Melee speed scales with round number
f32 PowerUpSystem::GetMeleeSpeedForRound(u32 round) const
{
    f32 baseSpeed = 250.0f;
    f32 speedPerRound = 10.0f;  // +10 speed per round
    f32 maxSpeed = 400.0f;      // Cap at 400
    
    f32 speed = baseSpeed + (round * speedPerRound);
    return speed > maxSpeed ? maxSpeed : speed;
}

// Melee damage scales with round number
f32 PowerUpSystem::GetMeleeDamageForRound(u32 round) const
{
    f32 baseDamage = 50.0f;
    f32 damagePerRound = 5.0f;  // +5 damage per round
    
    return baseDamage + (round * damagePerRound);
}
