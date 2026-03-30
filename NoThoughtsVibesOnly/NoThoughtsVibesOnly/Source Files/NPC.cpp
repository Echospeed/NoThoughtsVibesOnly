// ============================================================================
// NPC.cpp - Enemy AI Implementation
// ============================================================================
// Four enemy types, each with distinct movement and attack behaviour:
//
//   NPC_WALK   (Blue Ship)      : Wanders randomly. No shooting.
//   NPC_MELEE  (Green Ship)     : Charges directly at the player. Explodes on contact.
//   NPC_RANGER (Pink Ship)      : Keeps distance; fires single bullets.
//   NPC_BOSS   (Beige Ship)     : Orbits the player; fires 8-way bullet volleys.
//
// All stats (health, speed, scale, fireRate, colours, heal, burst counts, etc.)
// are now sourced entirely from GameConfig / npc_config.json.
// ============================================================================

#include "pch.hpp"
#include "NPC.hpp"
#include "NPCType.hpp"
#include "Player.hpp"
#include "GamePage.hpp"
#include <cmath>
#include <iostream>
#include "Bullet.hpp"
#include <random>
#include "PowerUpSystem.hpp"
#include "WaveSystem.hpp"

extern PowerUpSystem powerUpSystem; // Owned by GamePage.cpp
extern WaveSystem    waveSystem;    // Owned by GamePage.cpp


// ============================================================================
// Shared NPC texture cache
// ============================================================================
// Textures are loaded ONCE in Game_Load() via NPC_LoadTextures() and freed
// in Game_Unload() via NPC_UnloadTextures(). Each NPC::Start() points its
// spriteRenderer.texture at the appropriate cached pointer - no per-NPC load.
// Texture paths are read from npc_config.json via GameConfig::Npc().
// ============================================================================
static AEGfxTexture* s_TexWalk = nullptr;
static AEGfxTexture* s_TexMelee = nullptr;
static AEGfxTexture* s_TexRanger = nullptr;
static AEGfxTexture* s_TexBoss = nullptr;

void NPC_LoadTextures()
{
    const auto& nc = GameConfig::Npc();
    s_TexWalk = AEGfxTextureLoad(nc.walk.texture.c_str());
    s_TexMelee = AEGfxTextureLoad(nc.melee.texture.c_str());
    s_TexRanger = AEGfxTextureLoad(nc.ranger.texture.c_str());
    s_TexBoss = AEGfxTextureLoad(nc.boss.texture.c_str());
}

void NPC_UnloadTextures()
{
    if (s_TexWalk) { AEGfxTextureUnload(s_TexWalk);   s_TexWalk = nullptr; }
    if (s_TexMelee) { AEGfxTextureUnload(s_TexMelee);  s_TexMelee = nullptr; }
    if (s_TexRanger) { AEGfxTextureUnload(s_TexRanger); s_TexRanger = nullptr; }
    if (s_TexBoss) { AEGfxTextureUnload(s_TexBoss);   s_TexBoss = nullptr; }
}

// ============================================================================
// NPC Constructor
// ============================================================================
NPC::NPC()
{
    explosionParticles = ParticleSystem::MakeExplosion();
}

// ============================================================================
// NPC Destructor
// ============================================================================
NPC::~NPC()
{
    // Do NOT call AEGfxTextureUnload here - texture is shared, not owned by this NPC
    spriteRenderer.texture = nullptr;
    NPCSpritesheet = nullptr;
}

// ============================================================================
// Start
// ============================================================================
// Spawns the NPC at a random world position, then applies ALL type-specific
// stats from npc_config.json via GameConfig::Npc().
// ============================================================================
void NPC::Start()
{
    const auto& nc = GameConfig::Npc();

    // --- Random spawn: keep trying until we're far enough from the player ---
    f32 rX{ 0.0f }, rY{ 0.0f };
    AEVec2 spawnPos{};
    do
    {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<f32> distX(-WORLD_WIDTH / 2.0f, WORLD_WIDTH / 2.0f);
        std::uniform_real_distribution<f32> distY(-WORLD_HEIGHT / 2.0f, WORLD_HEIGHT / 2.0f);
        rX = distX(rng);
        rY = distY(rng);
        spawnPos = { rX, rY };
    } while (AEVec2Distance(&spawnPos, &target->transform.position) < GameConfig::Player().spawnClearRadius);
    transform.position = { rX, rY };
    transform.rotation = 0.0f;

    // Reset death/explosion state in case this NPC slot is reused
    hasExploded = false;

    // Re-initialise explosion system
    explosionParticles = ParticleSystem::MakeExplosion();

    // --- Type-specific setup: all values from GameConfig ---
    switch (type)
    {
    case NPC_WALK:
        NPCSpritesheet = s_TexWalk;
        spriteRenderer.texture = NPCSpritesheet;
        spriteRenderer.colour = { 1.0f, 1.0f, 1.0f, 1.0f };
        spriteRenderer.meshType = MESH_SQUARE;
        transform.scale = { nc.walk.scaleX, nc.walk.scaleY };
        health = nc.walk.health;
        speed = nc.walk.speed;
        baseColour = { nc.walk.colourR,   nc.walk.colourG,   nc.walk.colourB,   1.0f };
        break;

    case NPC_MELEE:
        NPCSpritesheet = s_TexMelee;
        spriteRenderer.texture = NPCSpritesheet;
        spriteRenderer.colour = { 1.0f, 1.0f, 1.0f, 1.0f };
        spriteRenderer.meshType = MESH_SQUARE;
        transform.scale = { nc.melee.scaleX, nc.melee.scaleY };
        health = nc.melee.health;
        speed = nc.melee.speed;
        baseColour = { nc.melee.colourR,  nc.melee.colourG,  nc.melee.colourB,  1.0f };
        break;

    case NPC_RANGER:
        NPCSpritesheet = s_TexRanger;
        spriteRenderer.texture = NPCSpritesheet;
        spriteRenderer.colour = { 1.0f, 1.0f, 1.0f, 1.0f };
        spriteRenderer.meshType = MESH_SQUARE;
        transform.scale = { nc.ranger.scaleX, nc.ranger.scaleY };
        health = nc.ranger.health;
        speed = nc.ranger.speed;
        fireRate = nc.ranger.fireRate;          // was hardcoded 2.0f
        baseColour = { nc.ranger.colourR, nc.ranger.colourG, nc.ranger.colourB, 1.0f };
        //std::cout << "[NPC] Ranger spawned at ("
        //    << transform.position.x << ", " << transform.position.y << ")\n";
        break;

    case NPC_BOSS:
        NPCSpritesheet = s_TexBoss;
        spriteRenderer.texture = NPCSpritesheet;
        spriteRenderer.colour = { 1.0f, 1.0f, 1.0f, 1.0f };
        spriteRenderer.meshType = MESH_SQUARE;
        transform.scale = { nc.boss.scaleX, nc.boss.scaleY };   // was hardcoded {150,150}
        health = nc.boss.health;                        // was hardcoded 1000.0f
        speed = nc.boss.speed;                         // was hardcoded 150.0f
        fireRate = nc.boss.fireRate;                      // was hardcoded 0.5f
        baseColour = { nc.boss.colourR,  nc.boss.colourG,  nc.boss.colourB,  1.0f };
        //std::cout << "[BOSS] Boss spawned at ("
        //    << transform.position.x << ", " << transform.position.y << ")\n";
        break;
    }

}

// ============================================================================
// Update
// ============================================================================
void NPC::Update(f32 deltaTime)
{
    // Always tick explosion particles regardless of death/visibility state.
    explosionParticles.Update(deltaTime);

    if (!isVisibleToPlayer) return;

    // --- Death check ---
    if (health <= 0.0f)
    {
        isVisibleToPlayer = false;
        isActive = false;
        spriteRenderer.colour.a = 0.0f;

        // Award XP - read from config per type
        const auto& nc = GameConfig::Npc();
        const f32 xpReward = (type == NPC_BOSS) ? nc.boss.xpReward
            : (type == NPC_MELEE) ? nc.melee.xpReward
            : (type == NPC_RANGER) ? nc.ranger.xpReward
            : nc.walk.xpReward;
        powerUpSystem.AddExperience(xpReward);

        // Award kill score
        const auto& sc = GameConfig::Gameplay();
        const int killScore = (type == NPC_BOSS) ? sc.killScoreBoss
            : (type == NPC_MELEE) ? sc.killScoreMelee
            : (type == NPC_RANGER) ? sc.killScoreRanger
            : sc.killScoreWalker;
        waveSystem.AddKillScore(killScore);

        // Heal the player on kill - base heal from config, capped at maxHealth
        if (target)
        {
            Player* player = dynamic_cast<Player*>(target);
            if (player)
            {
                const f32 baseHeal = (type == NPC_BOSS) ? nc.boss.baseHeal : nc.walk.baseHeal;
                const f32 healAmount = baseHeal + powerUpSystem.GetStats().lifestealBonus;
                player->health += healAmount;
                if (player->health > player->maxHealth)
                    player->health = player->maxHealth;
            }
        }

        // Explosion burst - count from config
        if (!hasExploded)
        {
            hasExploded = true;
            const int burstCount = (type == NPC_BOSS) ? nc.boss.burstCount : nc.walk.burstCount;
            explosionParticles.EmitBurst(transform.position, burstCount);
        }

        return;
    }

    // --- Dispatch to type-specific AI ---
    switch (type)
    {
    case NPC_WALK:   WalkNPCs(deltaTime);   break;
    case NPC_MELEE:  BomberNPCs(deltaTime); break;
    case NPC_RANGER: RangerNPCs(deltaTime); break;
    case NPC_BOSS:   BossNPCs(deltaTime);   break;
    }
}

// ============================================================================
// BomberNPCs (NPC_MELEE)
// ============================================================================
void NPC::BomberNPCs(f32 deltaTime)
{
    if (!target || health <= 0.0f) return;

    const auto& nc = GameConfig::Npc();

    AEVec2 dir{};
    AEVec2 disp = {
        target->transform.position.x - transform.position.x,
        target->transform.position.y - transform.position.y
    };
    AEVec2Normalize(&dir, &disp);

    transform.position.x += dir.x * speed * deltaTime;
    transform.position.y += dir.y * speed * deltaTime;

    // --- Wall bounce ---
    const f32 halfW = WORLD_WIDTH / 2.0f;
    const f32 halfH = WORLD_HEIGHT / 2.0f;
    const f32 hw = transform.scale.x / 2.0f;
    const f32 hh = transform.scale.y / 2.0f;

    if (transform.position.x > halfW - hw) { transform.position.x = halfW - hw; velocity.x = -velocity.x; }
    if (transform.position.x < -halfW + hw) { transform.position.x = -halfW + hw; velocity.x = -velocity.x; }
    if (transform.position.y > halfH - hh) { transform.position.y = halfH - hh; velocity.y = -velocity.y; }
    if (transform.position.y < -halfH + hh) { transform.position.y = -halfH + hh; velocity.y = -velocity.y; }

    // --- Contact explosion ---
    const f32 dx = target->transform.position.x - transform.position.x;
    const f32 dy = target->transform.position.y - transform.position.y;
    const f32 dist = sqrtf(dx * dx + dy * dy);
    const f32 playerRadius = (target->transform.scale.x + target->transform.scale.y) * 0.25f;
    const f32 selfRadius = (transform.scale.x + transform.scale.y) * 0.25f;

    if (dist < playerRadius + selfRadius)
    {
        Player* player = dynamic_cast<Player*>(target);
        if (player && !player->invulnAbility.IsActive())
        {
            player->health -= nc.melee.explosionDamage;
            if (player->health < 0.0f) player->health = 0.0f;
            //std::cout << "[Melee] Contact explosion! -" << nc.melee.explosionDamage << " HP to player\n";
        }

        if (!hasExploded)
        {
            hasExploded = true;
            explosionParticles.EmitBurst(transform.position, nc.melee.explosionBurst);
        }

        health = 0.0f;
    }
}

// ============================================================================
// RangerNPCs (NPC_RANGER)
// ============================================================================
void NPC::RangerNPCs(f32 deltaTime)
{
    if (!target || health <= 0.0f) return;

    const auto& rc = GameConfig::Npc().ranger;

    // --- Periodic random direction change ---
    changeDirTimer -= deltaTime;
    if (changeDirTimer <= 0.0f)
    {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<f32> dist(-1.0f, 1.0f);
        f32 randX = dist(rng);
        f32 randY = dist(rng);
        f32 len = sqrtf(randX * randX + randY * randY);
        if (len > 0.0f)
        {
            velocity.x = (randX / len) * speed;
            velocity.y = (randY / len) * speed;
        }
        changeDirTimer = rc.changeDirInterval;
    }

    // --- Movement ---
    transform.position.x += velocity.x * deltaTime;
    transform.position.y += velocity.y * deltaTime;

    AEVec2 toPlayer = {
        target->transform.position.x - transform.position.x,
        target->transform.position.y - transform.position.y
    };
    const f32 dist = sqrtf(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);

    if (dist < rc.retreatDistance)
    {
        velocity.x = -(toPlayer.x / dist) * speed;
        velocity.y = -(toPlayer.y / dist) * speed;
    }
    else
    {
        velocity.x *= rc.velocityDamping;
        velocity.y *= rc.velocityDamping;
    }

    //transform.position.x += velocity.x * deltaTime;
    //transform.position.y += velocity.y * deltaTime;

    // --- Wall bounce ---
    const f32 halfW = WORLD_WIDTH / 2.0f;
    const f32 halfH = WORLD_HEIGHT / 2.0f;
    const f32 hw = transform.scale.x / 2.0f;
    const f32 hh = transform.scale.y / 2.0f;

    if (transform.position.x > halfW - hw) { transform.position.x = halfW - hw; velocity.x = -velocity.x; }
    if (transform.position.x < -halfW + hw) { transform.position.x = -halfW + hw; velocity.x = -velocity.x; }
    if (transform.position.y > halfH - hh) { transform.position.y = halfH - hh; velocity.y = -velocity.y; }
    if (transform.position.y < -halfH + hh) { transform.position.y = -halfH + hh; velocity.y = -velocity.y; }

    // --- Shooting ---
    fireCooldown -= deltaTime;
    if (fireCooldown <= 0.0f)
    {
        for (auto& obj : gamePageObj)
        {
            if (obj->ObjectType != SHOT) continue;
            Bullet* b = dynamic_cast<Bullet*>(obj);
            if (!b || b->owner != BulletOwner::ENEMY) continue;
            if (b->isActive || b->startPos != this)   continue;

            AEVec2 dir = toPlayer;
            const f32 mag = sqrtf(dir.x * dir.x + dir.y * dir.y);
            if (mag > 0.0f) { dir.x /= mag; dir.y /= mag; }

            b->Activate(this, dir, BulletOwner::ENEMY);
            b->spriteRenderer.colour = { 1.0f, 0.0f, 0.0f, 1.0f };

            //std::cout << "[Ranger] Fired!\n";
            break;
        }
        fireCooldown = fireRate;
    }
}

// ============================================================================
// WalkNPCs (NPC_WALK)
// ============================================================================
void NPC::WalkNPCs(f32 deltaTime)
{
    if (!target || health <= 0.0f) return;

    const auto& wc = GameConfig::Npc().walk;

    changeDirTimer -= deltaTime;
    if (changeDirTimer <= 0.0f)
    {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<f32> dist(-1.0f, 1.0f);
        f32 randX = dist(rng);
        f32 randY = dist(rng);
        f32 len = sqrtf(randX * randX + randY * randY);
        if (len > 0.0f)
        {
            velocity.x = (randX / len) * speed;
            velocity.y = (randY / len) * speed;
        }
        changeDirTimer = wc.changeDirInterval;
    }

    transform.position.x += velocity.x * deltaTime;
    transform.position.y += velocity.y * deltaTime;

    // --- Wall bounce ---
    const f32 halfW = WORLD_WIDTH / 2.0f;
    const f32 halfH = WORLD_HEIGHT / 2.0f;
    const f32 hw = transform.scale.x / 2.0f;
    const f32 hh = transform.scale.y / 2.0f;

    if (transform.position.x > halfW - hw) { transform.position.x = halfW - hw; velocity.x = -velocity.x; }
    if (transform.position.x < -halfW + hw) { transform.position.x = -halfW + hw; velocity.x = -velocity.x; }
    if (transform.position.y > halfH - hh) { transform.position.y = halfH - hh; velocity.y = -velocity.y; }
    if (transform.position.y < -halfH + hh) { transform.position.y = -halfH + hh; velocity.y = -velocity.y; }
}

// ============================================================================
// BossNPCs (NPC_BOSS)
// ============================================================================
void NPC::BossNPCs(f32 deltaTime)
{
    if (!target || health <= 0.0f) return;

    const auto& bc = GameConfig::Npc().boss;

    // --- Orbit movement ---
    AEVec2 toPlayer = {
        target->transform.position.x - transform.position.x,
        target->transform.position.y - transform.position.y
    };
    const f32 dist = sqrtf(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
    const f32 orbitDist = bc.orbitDistance;   // was hardcoded 300.0f
    const f32 orbitMargin = bc.orbitMargin;      // was hardcoded 50.0f

    if (dist > 0.0f)
    {
        toPlayer.x /= dist;
        toPlayer.y /= dist;

        if (dist > orbitDist + orbitMargin)
        {
            velocity.x = toPlayer.x * speed;
            velocity.y = toPlayer.y * speed;
        }
        else if (dist < orbitDist - orbitMargin)
        {
            velocity.x = -toPlayer.x * speed;
            velocity.y = -toPlayer.y * speed;
        }
        else
        {
            velocity.x = -toPlayer.y * speed;
            velocity.y = toPlayer.x * speed;
        }
    }

    transform.position.x += velocity.x * deltaTime;
    transform.position.y += velocity.y * deltaTime;

    // --- Wall bounce ---
    const f32 halfW = WORLD_WIDTH / 2.0f;
    const f32 halfH = WORLD_HEIGHT / 2.0f;
    const f32 hw = transform.scale.x / 2.0f;
    const f32 hh = transform.scale.y / 2.0f;

    if (transform.position.x > halfW - hw) { transform.position.x = halfW - hw; velocity.x = -velocity.x; }
    if (transform.position.x < -halfW + hw) { transform.position.x = -halfW + hw; velocity.x = -velocity.x; }
    if (transform.position.y > halfH - hh) { transform.position.y = halfH - hh; velocity.y = -velocity.y; }
    if (transform.position.y < -halfH + hh) { transform.position.y = -halfH + hh; velocity.y = -velocity.y; }

    // --- 8-way bullet volley (volley count from config) ---
    fireCooldown -= deltaTime;
    if (fireCooldown <= 0.0f)
    {
        int bulletsFired = 0;
        const int volleyCount = bc.bulletVolleyCount;   // was hardcoded 8

        for (auto& obj : gamePageObj)
        {
            if (obj->ObjectType != SHOT) continue;
            Bullet* b = dynamic_cast<Bullet*>(obj);
            if (!b || b->owner != BulletOwner::ENEMY) continue;
            if (b->isActive || b->startPos != this)   continue;

            const f32 baseAngle = atan2f(
                target->transform.position.y - transform.position.y,
                target->transform.position.x - transform.position.x
            );
            const f32  angle = baseAngle + bulletsFired * (2.0f * 3.14159f / volleyCount);
            const AEVec2 dir = { cosf(angle), sinf(angle) };

            b->Activate(this, dir, BulletOwner::ENEMY);
            b->spriteRenderer.colour = { 1.0f, 0.0f, 1.0f, 1.0f };

            ++bulletsFired;
            if (bulletsFired >= volleyCount) break;
        }

        //if (bulletsFired > 0)
        //    std::cout << "[BOSS] Fired " << bulletsFired << " bullets!\n";

        fireCooldown = fireRate;
    }

}