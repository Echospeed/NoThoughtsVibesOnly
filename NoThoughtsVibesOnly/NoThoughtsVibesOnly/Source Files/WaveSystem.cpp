// ============================================================================
// WaveSystem.cpp - FIXED VERSION (No Warnings)
// ============================================================================
// COPY AND PASTE THIS ENTIRE FILE
// 
// FIX: Renamed local 'availableBullets' to 'availableEnemyBullets' 
//      to avoid shadowing global declaration (Line 229)
// ============================================================================

#include "pch.hpp"
#include "WaveSystem.hpp"
#include "GamePage.hpp"
#include "Player.hpp"
#include "NPC.hpp"
#include "Bullet.hpp"
#include <iostream>

void WaveSystem::Init(Player* player)
{
    playerRef = player;
    currentWave = 0;
    currentRound = 1;
    inWaveBreak = false;
    waveBreakTimer = 0.0f;

    std::cout << "\n========================================\n";
    std::cout << "    WAVE SYSTEM INITIALIZED\n";
    std::cout << "    Press C to start first wave!\n";
    std::cout << "========================================\n\n";
}

void WaveSystem::Update(f32 deltaTime)
{
    if (inWaveBreak)
    {
        waveBreakTimer -= deltaTime;

        if (waveBreakTimer <= 0.0f)
        {
            StartNextWave();
        }

        return;
    }

    if (currentWave > 0 && IsWaveComplete())
    {
        std::cout << "\n========================================\n";
        std::cout << "    WAVE " << currentWave << " CLEARED!\n";
        std::cout << "    Next wave in " << WAVE_BREAK_DURATION << " seconds...\n";
        std::cout << "========================================\n\n";

        currentRound++;
        inWaveBreak = true;
        waveBreakTimer = WAVE_BREAK_DURATION;
    }
}

void WaveSystem::StartNextWave()
{
    currentWave++;
    inWaveBreak = false;

    WaveConfig config = GenerateWaveConfig();

    std::cout << "\n========================================\n";
    std::cout << "    WAVE " << currentWave << " (Round " << currentRound << ")\n";
    std::cout << "    Walkers: " << config.walkerCount << "\n";
    std::cout << "    Melee: " << config.meleeCount << "\n";
    std::cout << "    Rangers: " << config.rangerCount << "\n";
    if (config.hasBoss)
    {
        std::cout << "    WARNING: BOSS WAVE!\n";
    }
    std::cout << "========================================\n\n";

    SpawnWave(config);
}

bool WaveSystem::IsWaveComplete() const
{
    for (GameObject* obj : gamePageObj)
    {
        if (obj && obj->isActive && obj->ObjectType == NP)
        {
            return false;
        }
    }

    return true;
}

WaveConfig WaveSystem::GenerateWaveConfig() const
{
    WaveConfig config{};

    u32 baseCount = GetEnemyCountForRound();

    if (currentRound <= 3)
    {
        config.walkerCount = baseCount;
        config.meleeCount = currentRound;
        config.rangerCount = 0;
    }
    else if (currentRound <= 7)
    {
        config.walkerCount = baseCount / 2;
        config.meleeCount = baseCount / 3;
        config.rangerCount = baseCount / 4;
    }
    else
    {
        config.walkerCount = baseCount / 3;
        config.meleeCount = baseCount / 3;
        config.rangerCount = baseCount / 2;
    }

    config.hasBoss = ShouldSpawnBoss();
    config.expReward = 50.0f + (currentRound * 25.0f);

    return config;
}

void WaveSystem::SpawnWave(const WaveConfig& config)
{
    if (!playerRef) return;

    // Spawn Walker Enemies
    for (u32 i = 0; i < config.walkerCount; ++i)
    {
        NPC* n = new NPC();
        n->ObjectType = NP;
        n->type = NPC_WALK;
        n->target = playerRef;
        n->Start();
    }

    // Spawn Melee Enemies
    for (u32 i = 0; i < config.meleeCount; ++i)
    {
        NPC* n = new NPC();
        n->ObjectType = NP;
        n->type = NPC_MELEE;
        n->target = playerRef;
        n->Start();
    }

    // Spawn Ranger Enemies
    for (u32 i = 0; i < config.rangerCount; ++i)
    {
        NPC* n = new NPC();
        n->ObjectType = NP;
        n->type = NPC_RANGER;
        n->target = playerRef;
        n->Start();

        // ====================================================================
        // FIX: Renamed variable to avoid shadowing global 'availableBullets'
        // ====================================================================
        u32 bulletsAssigned = 0;
        int totalBullets = 0;
        int enemyBullets = 0;
        int availableEnemyBullets = 0;  // FIXED: Was 'availableBullets'

        for (auto& obj : gamePageObj)
        {
            if (obj->ObjectType == SHOT)
            {
                totalBullets++;
                Bullet* b = dynamic_cast<Bullet*>(obj);
                if (b && b->owner == BulletOwner::ENEMY)
                {
                    enemyBullets++;
                    if (b->startPos == nullptr)
                    {
                        availableEnemyBullets++;
                        b->startPos = n;
                        b->isActive = false;
                        bulletsAssigned++;
                        if (bulletsAssigned >= 3) break;
                    }
                }
            }
        }

        std::cout << "[WAVE] Ranger #" << (i + 1) << " spawned with " << bulletsAssigned
            << " bullets (Total: " << totalBullets
            << ", Enemy: " << enemyBullets
            << ", Available: " << availableEnemyBullets << ")\n";
    }

    if (config.hasBoss)
    {
        SpawnBoss();
    }

    std::cout << "[WAVE SPAWN] Spawned " << config.walkerCount + config.meleeCount + config.rangerCount
        << " enemies" << (config.hasBoss ? " + BOSS" : "") << "\n";
}

void WaveSystem::SpawnBoss()
{
    if (!playerRef) return;

    std::cout << "\n=======================================\n";
    std::cout << "   BOSS WAVE - INCOMING!\n";
    std::cout << "=======================================\n\n";

    NPC* boss = new NPC();
    boss->ObjectType = NP;
    boss->type = NPC_BOSS;
    boss->target = playerRef;
    boss->Start();

    u32 bulletsAssigned = 0;
    for (auto& obj : gamePageObj)
    {
        if (obj->ObjectType == SHOT)
        {
            Bullet* b = dynamic_cast<Bullet*>(obj);
            if (b && b->owner == BulletOwner::ENEMY && b->startPos == nullptr)
            {
                b->startPos = boss;
                bulletsAssigned++;
                if (bulletsAssigned >= 8) break;
            }
        }
    }

    std::cout << "[BOSS SPAWN] Boss spawned with " << bulletsAssigned << " bullets!\n";
    std::cout << "[BOSS STATS] Health: 1000 | Speed: 150 | Fire Rate: 0.5s\n";
    std::cout << "[BOSS ATTACK] Shoots 8 bullets in all directions!\n\n";
}

u32 WaveSystem::GetEnemyCountForRound() const
{
    u32 count = 5 + (currentRound * 2);

    if (count > 20) count = 20;

    return count;
}

bool WaveSystem::ShouldSpawnBoss() const
{
    return (currentRound % 5) == 0;
}

void WaveSystem::Cleanup()
{
    playerRef = nullptr;
    currentWave = 0;
    currentRound = 1;
    inWaveBreak = false;
    waveBreakTimer = 0.0f;
}
