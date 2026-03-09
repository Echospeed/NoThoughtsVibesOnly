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
//
// EXPLOSION PARTICLE LIFECYCLE:
// ----------------------------------------------------------------------------
//   Constructor : explosionParticles is configured via MakeExplosion() preset
//   Start()     : explosionParticles is re-initialised (in case of reuse)
//   Update()    : explosionParticles.Update() runs EVERY frame, even after death,
//                 so the burst can finish playing out after the NPC is removed.
//                 The guard is: if (!isActive && !hasExploded) skip AI, else tick particles.
//   Death block : EmitBurst fires all particles at once, hasExploded = true.
//   GamePage    : Render() is called when hasExploded==true regardless of isActive.
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
extern Audio* shootSFX;             // Declared in GamePage.cpp

// ============================================================================
// Shared NPC texture cache
// ============================================================================
// Textures are loaded ONCE in Game_Load() via NPC_LoadTextures() and freed
// in Game_Unload() via NPC_UnloadTextures(). Each NPC::Start() points its
// spriteRenderer.texture at the appropriate cached pointer - no per-NPC load.
// ============================================================================
static AEGfxTexture* s_TexWalk = nullptr;
static AEGfxTexture* s_TexMelee = nullptr;
static AEGfxTexture* s_TexRanger = nullptr;
static AEGfxTexture* s_TexBoss = nullptr;

void NPC_LoadTextures()
{
    // Load each ship texture once for the whole game session
    s_TexWalk = AEGfxTextureLoad("Assets/shipBlue_manned.png");
    s_TexMelee = AEGfxTextureLoad("Assets/shipGreen_manned.png");
    s_TexRanger = AEGfxTextureLoad("Assets/shipPink_manned.png");
    s_TexBoss = AEGfxTextureLoad("Assets/shipBeige_manned.png");
}

void NPC_UnloadTextures()
{
    // Null-check before unloading - safe to call even if Load was never called
    if (s_TexWalk) { AEGfxTextureUnload(s_TexWalk);   s_TexWalk = nullptr; }
    if (s_TexMelee) { AEGfxTextureUnload(s_TexMelee);  s_TexMelee = nullptr; }
    if (s_TexRanger) { AEGfxTextureUnload(s_TexRanger); s_TexRanger = nullptr; }
    if (s_TexBoss) { AEGfxTextureUnload(s_TexBoss);   s_TexBoss = nullptr; }
}

// ============================================================================
// NPC Constructor
// ============================================================================
// Sets up the explosion particle system using the MakeExplosion preset.
// This runs before Start() so the system is ready by the time the NPC spawns.
// ============================================================================
NPC::NPC()
{
    explosionParticles = ParticleSystem::MakeExplosion();
}

// ============================================================================
// NPC Destructor
// ============================================================================
// Nulls the texture pointer without unloading it - the texture is owned by
// the shared cache (s_TexWalk etc.) and freed in NPC_UnloadTextures().
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
// Spawns the NPC at a random world position at least 100 units from the player.
// Sets type-specific appearance, stats, and re-initialises explosion particles.
// Called by WaveSystem after setting NPC::type and NPC::target.
// ============================================================================
void NPC::Start()
{
    // --- Random spawn: keep trying until we're far enough from the player ---
    f32    rX, rY;
    AEVec2 spawnPos{};
    do
    {
        rX = ((f32)rand() / RAND_MAX) * WORLD_WIDTH - WORLD_WIDTH / 2.0f;
        rY = ((f32)rand() / RAND_MAX) * WORLD_HEIGHT - WORLD_HEIGHT / 2.0f;
        spawnPos = { rX, rY };
    } while (AEVec2Distance(&spawnPos, &target->transform.position) < 100.0f);

    transform.position = { rX, rY };
    transform.scale = { 50.0f, 50.0f }; // Same size as player by default
    transform.rotation = 0.0f;

    // Reset death/explosion state in case this NPC slot is reused
    hasExploded = false;

    // Re-initialise explosion system - ensures a clean burst even if Start() is called twice
    explosionParticles = ParticleSystem::MakeExplosion();

    // --- Type-specific appearance: point texture at the shared cached texture ---
    switch (type)
    {
    case NPC_WALK:
        NPCSpritesheet = s_TexWalk; // Shared - do not free in destructor
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
        transform.scale = { 150.0f, 150.0f }; // 3x player size
        spriteRenderer.colour = { 1.0f, 1.0f, 1.0f, 1.0f };
        spriteRenderer.meshType = MESH_SQUARE;
        baseColour = { 1.0f, 1.0f, 1.0f, 1.0f };
        health = 1000.0f; // 10x normal health
        speed = 150.0f;  // Slightly slower
        fireRate = 0.5f;    // Fires twice per second
        std::cout << "[BOSS] Boss spawned at ("
            << transform.position.x << ", " << transform.position.y << ")\n";
        break;
    }
}

// ============================================================================
// Update
// ============================================================================
// Called each frame from GamePage Game_Update().
//
// KEY DESIGN: explosionParticles.Update() runs unconditionally at the top,
// BEFORE any isActive/isVisibleToPlayer guards. This is intentional:
//   - When the NPC dies, isActive becomes false and isVisibleToPlayer becomes false.
//   - GamePage only calls Update() when isActive==true (see Game_Update).
//   - But GamePage.cpp calls npc->Update(dt) only if isActive.
//   - So we need particles to update even after death.
//
// SOLUTION: GamePage.cpp must call npc->Update(dt) OR npc->explosionParticles.Update(dt)
// even when isActive==false. The cleanest approach is to always call Update()
// for NPCs regardless of isActive, and guard the AI inside this function.
// See the note in GamePage.cpp Game_Update() section 7.
// ============================================================================
void NPC::Update(f32 deltaTime)
{
    // Always tick explosion particles regardless of death/visibility state.
    // The burst is fired on the death frame; subsequent frames let it play out.
    explosionParticles.Update(deltaTime);

    // If the NPC is dead (invisible), skip all AI - just let particles finish
    if (!isVisibleToPlayer) return;

    // --- Death check ---
    if (health <= 0.0f)
    {
        // Hide and deactivate the NPC sprite
        isVisibleToPlayer = false;
        isActive = false;
        spriteRenderer.colour.a = 0.0f;

        // Award XP to the player
        powerUpSystem.AddExperience(25.0f);

        // Fire the explosion burst - EmitBurst spawns all particles at once.
        // Boss gets 40 (fills the pool), normal enemies get 20.
        if (!hasExploded)
        {
            hasExploded = true; // Mark so GamePage.cpp knows to call Render()
            const int burstCount = (type == NPC_BOSS) ? 40 : 20;
            explosionParticles.EmitBurst(transform.position, burstCount);

            if (type == NPC_BOSS)
                std::cout << "[BOSS] Boss defeated!\n";
        }

        return; // Skip AI this frame
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

    if (transform.position.x > halfW - hw) { transform.position.x = halfW - hw;  velocity.x = -velocity.x; }
    if (transform.position.x < -halfW + hw) { transform.position.x = -halfW + hw;  velocity.x = -velocity.x; }
    if (transform.position.y > halfH - hh) { transform.position.y = halfH - hh;  velocity.y = -velocity.y; }
    if (transform.position.y < -halfH + hh) { transform.position.y = -halfH + hh;  velocity.y = -velocity.y; }
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
        // Too close - retreat directly away from player
        velocity.x = -(toPlayer.x / dist) * speed;
        velocity.y = -(toPlayer.y / dist) * speed;
    }
    else
    {
        // Gradually slow random wander so ranger doesn't rocket off
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

    if (transform.position.x > halfW - hw) { transform.position.x = halfW - hw;  velocity.x = -velocity.x; }
    if (transform.position.x < -halfW + hw) { transform.position.x = -halfW + hw;  velocity.x = -velocity.x; }
    if (transform.position.y > halfH - hh) { transform.position.y = halfH - hh;  velocity.y = -velocity.y; }
    if (transform.position.y < -halfH + hh) { transform.position.y = -halfH + hh;  velocity.y = -velocity.y; }

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

            // Aim directly at the player using the cached toPlayer direction
            AEVec2 dir = toPlayer;
            const f32 mag = sqrtf(dir.x * dir.x + dir.y * dir.y);
            if (mag > 0.0f) { dir.x /= mag; dir.y /= mag; }

            b->Activate(this, dir, BulletOwner::ENEMY);
            b->spriteRenderer.colour = { 1.0f, 0.0f, 0.0f, 1.0f }; // Red bullet

            std::cout << "[Ranger] Fired!\n";
            break; // One bullet per fire tick
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

    // Pick a new random direction every changeDirTimer seconds
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

    if (transform.position.x > halfW - hw) { transform.position.x = halfW - hw;  velocity.x = -velocity.x; }
    if (transform.position.x < -halfW + hw) { transform.position.x = -halfW + hw;  velocity.x = -velocity.x; }
    if (transform.position.y > halfH - hh) { transform.position.y = halfH - hh;  velocity.y = -velocity.y; }
    if (transform.position.y < -halfH + hh) { transform.position.y = -halfH + hh;  velocity.y = -velocity.y; }
}

// ============================================================================
// BossNPCs (NPC_BOSS)
// ============================================================================
// Orbits the player at ~300 units, adjusting velocity to maintain distance:
//   dist > orbitDist + margin : Moves straight toward player (closing in)
//   dist < orbitDist - margin : Backs away (too close)
//   otherwise                 : Moves perpendicular (circular orbit)
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
        toPlayer.x /= dist; // Normalise in-place
        toPlayer.y /= dist;

        if (dist > orbitDist + orbitMargin)
        {
            // Too far - close in directly
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
            // In orbit band - move perpendicular for circular motion
            // Rotating toPlayer 90deg: (-y, x)
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

    if (transform.position.x > halfW - hw) { transform.position.x = halfW - hw;  velocity.x = -velocity.x; }
    if (transform.position.x < -halfW + hw) { transform.position.x = -halfW + hw;  velocity.x = -velocity.x; }
    if (transform.position.y > halfH - hh) { transform.position.y = halfH - hh;  velocity.y = -velocity.y; }
    if (transform.position.y < -halfH + hh) { transform.position.y = -halfH + hh;  velocity.y = -velocity.y; }

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

            // Evenly distribute 8 bullets around a full circle (360/8 = 45 deg each)
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