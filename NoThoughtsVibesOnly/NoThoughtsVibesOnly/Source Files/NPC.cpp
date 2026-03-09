// ============================================================================
// NPC.cpp - Enemy AI Implementation
// ============================================================================
// Four enemy types, each with distinct movement and attack behaviour:
//
//   NPC_WALK   (Blue Ship)      : Wanders randomly. No shooting.
//   NPC_MELEE  (Green Ship)     : Charges directly at the player. No shooting.
//   NPC_RANGER (Pink Ship)      : Keeps distance; fires single bullets.
//   NPC_BOSS   (Beige Ship)     : Orbits the player; fires 8-way bullet volleys.
//
// INTERACTION WITH WAVESYSTEM:
// ----------------------------------------------------------------------------
//   WaveSystem::SpawnWave() creates NPC instances with new NPC() and calls
//   NPC::Start() to randomise position and set type-specific stats.
//   Enemy bullets from the pool are assigned to Ranger/Boss in WaveSystem.
//
// INTERACTION WITH POWERSYSTEM:
// ----------------------------------------------------------------------------
//   On death, the NPC awards XP via powerUpSystem.AddExperience().
// ============================================================================

#include "pch.hpp"
#include "NPC.hpp"
#include "NPCType.hpp"
#include "Player.hpp"
#include "GamePage.hpp"
#include <math.h>
#include <iostream>
#include "Bullet.hpp"
#include "PowerUpSystem.hpp"

extern PowerUpSystem powerUpSystem; // Owned by GamePage.cpp
extern Audio* shootSFX;      // TODO: play on enemy attack

// ============================================================================
// Texture cache - loaded once, shared across all NPC instances of each type.
// Prevents a new AEGfxTextureLoad() call per NPC spawn (major leak source).
// Call NPC_LoadTextures() in Game_Load() and NPC_UnloadTextures() in Game_Unload().
// ============================================================================
static AEGfxTexture* s_TexWalk = nullptr;
static AEGfxTexture* s_TexMelee = nullptr;
static AEGfxTexture* s_TexRanger = nullptr;
static AEGfxTexture* s_TexBoss = nullptr;

void NPC_LoadTextures()
{
    s_TexWalk = AEGfxTextureLoad("Assets/shipBlue_manned.png");
    s_TexMelee = AEGfxTextureLoad("Assets/shipGreen_manned.png");
    s_TexRanger = AEGfxTextureLoad("Assets/shipPink_manned.png");
    s_TexBoss = AEGfxTextureLoad("Assets/shipBeige_manned.png");
}

void NPC_UnloadTextures()
{
    if (s_TexWalk) { AEGfxTextureUnload(s_TexWalk);   s_TexWalk = nullptr; }
    if (s_TexMelee) { AEGfxTextureUnload(s_TexMelee);  s_TexMelee = nullptr; }
    if (s_TexRanger) { AEGfxTextureUnload(s_TexRanger); s_TexRanger = nullptr; }
    if (s_TexBoss) { AEGfxTextureUnload(s_TexBoss);   s_TexBoss = nullptr; }
}

// ============================================================================
// Start
// ============================================================================
// Spawns the NPC at a random world position that is at least 100 units from
// the player. Sets appearance and stats based on NPC type.
// ============================================================================
void NPC::Start()
{
    // --- Random spawn point, guaranteed away from the player ---
    f32    rX, rY;
    AEVec2 spawnPos{};
    do
    {
        rX = ((f32)rand() / RAND_MAX) * WORLD_WIDTH - WORLD_WIDTH / 2.0f;
        rY = ((f32)rand() / RAND_MAX) * WORLD_HEIGHT - WORLD_HEIGHT / 2.0f;
        spawnPos = { rX, rY };
    } while (AEVec2Distance(&spawnPos, &target->transform.position) < 100.0f);

    transform.position = { rX, rY };
    transform.scale = { 50.0f, 50.0f };  // Same size as player
    transform.rotation = 0.0f;

    // --- Type-specific appearance and stats ---
    // Use shared texture cache - no per-instance AEGfxTextureLoad calls
    switch (type)
    {
    case NPC_WALK:
        NPCSpritesheet = s_TexWalk;
        spriteRenderer.texture = NPCSpritesheet;
        spriteRenderer.colour = { 1.0f, 1.0f, 1.0f, 1.0f };
        spriteRenderer.meshType = MESH_SQUARE;
        baseColour = { 1.0f, 1.0f, 1.0f, 1.0f };
        break;

    case NPC_MELEE:
        NPCSpritesheet = s_TexMelee;
        spriteRenderer.texture = NPCSpritesheet;
        spriteRenderer.colour = { 1.0f, 1.0f, 1.0f, 1.0f };
        spriteRenderer.meshType = MESH_SQUARE;
        baseColour = { 1.0f, 1.0f, 1.0f, 1.0f };
        break;

    case NPC_RANGER:
        NPCSpritesheet = s_TexRanger;
        spriteRenderer.texture = NPCSpritesheet;
        spriteRenderer.colour = { 1.0f, 1.0f, 1.0f, 1.0f };
        spriteRenderer.meshType = MESH_SQUARE;
        baseColour = { 1.0f, 1.0f, 1.0f, 1.0f };
        std::cout << "[NPC] Ranger spawned at ("
            << transform.position.x << ", " << transform.position.y << ")\n";
        break;

    case NPC_BOSS:
        NPCSpritesheet = s_TexBoss;
        spriteRenderer.texture = NPCSpritesheet;
        transform.scale = { 150.0f, 150.0f };
        spriteRenderer.colour = { 1.0f, 1.0f, 1.0f, 1.0f };
        spriteRenderer.meshType = MESH_SQUARE;
        baseColour = { 1.0f, 1.0f, 1.0f, 1.0f };
        health = 1000.0f;
        speed = 150.0f;
        fireRate = 0.5f;
        std::cout << "[BOSS] Boss spawned at ("
            << transform.position.x << ", " << transform.position.y << ")\n";
        break;
    }
}

// ============================================================================
// Update
// ============================================================================
// Called each frame from GamePage Game_Update().
// Skips logic if invisible to player (not yet discovered / dead).
// On death: awards XP, hides the NPC, and deactivates it.
// ============================================================================
void NPC::Update(f32 deltaTime)
{
    if (!isVisibleToPlayer) return;

    // --- Death check ---
    if (health <= 0.0f)
    {
        isVisibleToPlayer = false;
        isActive = false;
        spriteRenderer.colour.a = 0.0f;
        powerUpSystem.AddExperience(25.0f); // Award XP to player

        if (type == NPC_BOSS)
            std::cout << "[BOSS] Boss defeated!\n";

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
// Charges directly toward the player at full speed.
// Bounces off world boundaries.
// ============================================================================
void NPC::BomberNPCs(f32 deltaTime)
{
    if (!target || health <= 0.0f) return;

    // Move straight toward the player
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
}

// ============================================================================
// RangerNPCs (NPC_RANGER)
// ============================================================================
// Maintains distance from the player (retreats when closer than 250 units).
// Changes random wander direction every 2 seconds.
// Fires one bullet from its assigned pool at the player on fireCooldown.
// ============================================================================
void NPC::RangerNPCs(f32 deltaTime)
{
    if (!target || health <= 0.0f) return;

    // --- Periodic random direction change ---
    changeDirTimer -= deltaTime;
    if (changeDirTimer <= 0.0f)
    {
        f32 randX = ((f32)rand() / RAND_MAX) * 2.0f - 1.0f;
        f32 randY = ((f32)rand() / RAND_MAX) * 2.0f - 1.0f;
        f32 len = sqrtf(randX * randX + randY * randY);

        if (len > 0.0f)
        {
            velocity.x = (randX / len) * speed;
            velocity.y = (randY / len) * speed;
        }
        changeDirTimer = 2.0f;
    }

    // --- Movement: apply velocity, then override if too close to player ---
    transform.position.x += velocity.x * deltaTime;
    transform.position.y += velocity.y * deltaTime;

    AEVec2 toPlayer = {
        target->transform.position.x - transform.position.x,
        target->transform.position.y - transform.position.y
    };
    const f32 dist = sqrtf(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);

    if (dist < 250.0f)
    {
        // Too close - retreat
        velocity.x = -(toPlayer.x / dist) * speed;
        velocity.y = -(toPlayer.y / dist) * speed;
    }
    else
    {
        // Gradually slow random wander
        velocity.x *= 0.95f;
        velocity.y *= 0.95f;
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

    // --- Shooting: find an inactive bullet assigned to this NPC ---
    fireCooldown -= deltaTime;
    if (fireCooldown <= 0.0f)
    {
        for (auto& obj : gamePageObj)
        {
            if (obj->ObjectType != SHOT) continue;

            Bullet* b = dynamic_cast<Bullet*>(obj);
            if (!b || b->owner != BulletOwner::ENEMY) continue;
            if (b->isActive || b->startPos != this)   continue;

            // Aim directly at the player
            AEVec2 dir = toPlayer;
            const f32 mag = sqrtf(dir.x * dir.x + dir.y * dir.y);
            if (mag > 0.0f) { dir.x /= mag; dir.y /= mag; }

            b->Activate(this, dir, BulletOwner::ENEMY);
            b->spriteRenderer.colour = { 1.0f, 0.0f, 0.0f, 1.0f }; // Red

            std::cout << "[Ranger] Fired!\n";
            break;
        }
        fireCooldown = fireRate;
    }
}

// ============================================================================
// WalkNPCs (NPC_WALK)
// ============================================================================
// Wanders randomly by picking a new velocity direction every 2 seconds.
// Bounces off world boundaries.
// ============================================================================
void NPC::WalkNPCs(f32 deltaTime)
{
    if (!target || health <= 0.0f) return;

    changeDirTimer -= deltaTime;
    if (changeDirTimer <= 0.0f)
    {
        f32 randX = ((f32)rand() / RAND_MAX) * 2.0f - 1.0f;
        f32 randY = ((f32)rand() / RAND_MAX) * 2.0f - 1.0f;
        f32 len = sqrtf(randX * randX + randY * randY);

        if (len > 0.0f)
        {
            velocity.x = (randX / len) * speed;
            velocity.y = (randY / len) * speed;
        }
        changeDirTimer = 2.0f;
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
// Orbits the player at ~300 units, adjusting velocity to maintain distance:
//   - Too far  : Moves straight toward player.
//   - Too close: Backs away.
//   - In range : Moves perpendicular (orbits).
//
// Fires 8 bullets in a ring pattern every fireRate seconds.
// ============================================================================
void NPC::BossNPCs(f32 deltaTime)
{
    if (!target || health <= 0.0f) return;

    // --- Orbit movement ---
    AEVec2 toPlayer = {
        target->transform.position.x - transform.position.x,
        target->transform.position.y - transform.position.y
    };
    const f32 dist = sqrtf(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);
    const f32 orbitDist = 300.0f;
    const f32 orbitMargin = 50.0f;

    if (dist > 0.0f)
    {
        toPlayer.x /= dist; // Normalise
        toPlayer.y /= dist;

        if (dist > orbitDist + orbitMargin)
        {
            // Too far - close in
            velocity.x = toPlayer.x * speed;
            velocity.y = toPlayer.y * speed;
        }
        else if (dist < orbitDist - orbitMargin)
        {
            // Too close - back off
            velocity.x = -toPlayer.x * speed;
            velocity.y = -toPlayer.y * speed;
        }
        else
        {
            // In orbit range - move perpendicular for circular motion
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

    // --- 8-way bullet volley ---
    fireCooldown -= deltaTime;
    if (fireCooldown <= 0.0f)
    {
        int bulletsFired = 0;

        for (auto& obj : gamePageObj)
        {
            if (obj->ObjectType != SHOT) continue;

            Bullet* b = dynamic_cast<Bullet*>(obj);
            if (!b || b->owner != BulletOwner::ENEMY) continue;
            if (b->isActive || b->startPos != this)   continue;

            // Evenly distribute 8 bullets around a full circle
            const f32 angle = bulletsFired * (2.0f * 3.14159f / 8.0f);
            const AEVec2 dir = { cosf(angle), sinf(angle) };

            b->Activate(this, dir, BulletOwner::ENEMY);
            b->spriteRenderer.colour = { 1.0f, 0.0f, 1.0f, 1.0f }; // Magenta

            ++bulletsFired;
            if (bulletsFired >= 8) break;
        }

        if (bulletsFired > 0)
            std::cout << "[BOSS] Fired " << bulletsFired << " bullets!\n";

        fireCooldown = fireRate;
    }
}

// ============================================================================
// Destructor
// ============================================================================
// NPC textures are shared (loaded once via NPC_LoadTextures).
// Null out the pointers so ~GameObject() / FreeSpriteRenderer() does NOT
// try to unload the shared texture - NPC_UnloadTextures() handles that.
// ============================================================================
NPC::~NPC()
{
    spriteRenderer.texture = nullptr;
    NPCSpritesheet = nullptr;
}