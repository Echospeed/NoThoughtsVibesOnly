// ============================================================================
// Player.cpp - Player Controller Implementation
// ============================================================================
// See Player.hpp for class declaration and usage.
//
// CONTROLS:
// ----------------------------------------------------------------------------
//   W / A / S / D   : Move
//   Q / E           : Rotate
//   Left Mouse Hold : Shoot toward cursor (10 shots/sec)
//
// SYSTEMS USED:
// ----------------------------------------------------------------------------
//   PowerUpSystem  : Provides upgraded speed, damage, AoE radius/damage
//   Bullet Pool    : Player fires by reactivating pooled Bullet objects
//   NPC Collision  : AoE damage ring + contact knockback/damage
//   World Boundary : Player is clamped inside WORLD_WIDTH x WORLD_HEIGHT
// ============================================================================

#include "pch.hpp"
#include "Player.hpp"
#include "AEEngine.h"
#include <iostream>
#include "Bullet.hpp"
#include "NPC.hpp"
#include "GamePage.hpp"
#include <math.h>
#include "Input.hpp"
#include "PowerUpSystem.hpp"

extern Audio* shootSFX; // Declared and owned by GamePage.cpp

// ============================================================================
// Start
// ============================================================================
// Initialises the player at the world center with default stats.
// Called once from Game_Init() via obj->Start().
// ============================================================================
void Player::Start()
{
    transform.position = { 0.0f, 0.0f };
    transform.scale = { 50.0f, 50.0f };
    transform.rotation = 0.0f;

    // Red square appearance
    spriteRenderer.colour = { 1.0f, 0.0f, 0.0f, 1.0f };

    // Small initial cooldown prevents accidental shots at spawn
    shootCooldown = 0.3f;

    smokePS.Load();
    smokePS.Init(
        30,           // maxParticles — small puff, doesn't need many
        -30, 30,      // minVelX, maxVelX — spreads slightly sideways
        20, 80,       // minVelY, maxVelY — drifts upward
        0.6f,         // maxLifetime — short-lived puff
        12.0f          // size — smaller than the player itself
    );
}

// ============================================================================
// Update
// ============================================================================
// Runs every frame. Order of operations:
//   1. Read movement input (WASD) using current (upgraded) speed
//   2. Read rotation input (QE)
//   3. Compute mouse direction for aiming
//   4. Fire bullet on left-click hold (rate-limited by shootCooldown)
//   5. Apply AoE damage + contact damage to nearby NPCs
//   6. Clamp position to world boundaries
// ============================================================================
void Player::Update(f32 deltaTime)
{
    // ------------------------------------------------------------------
    // 1. Movement - speed sourced from PowerUpSystem for upgrade support
    // ------------------------------------------------------------------
    const f32 currentSpeed = GetSpeed();

    if (AEInputCheckCurr(AEVK_W)) transform.position.y += currentSpeed * deltaTime;
    if (AEInputCheckCurr(AEVK_S)) transform.position.y -= currentSpeed * deltaTime;
    if (AEInputCheckCurr(AEVK_A)) transform.position.x -= currentSpeed * deltaTime;
    if (AEInputCheckCurr(AEVK_D)) transform.position.x += currentSpeed * deltaTime;

    // ------------------------------------------------------------------
    // 2. Rotation
    // ------------------------------------------------------------------
    if (AEInputCheckCurr(AEVK_Q)) transform.rotation -= rotationSpeed * deltaTime;
    if (AEInputCheckCurr(AEVK_E)) transform.rotation += rotationSpeed * deltaTime;

    // ------------------------------------------------------------------
    // 3. Aim direction - from player toward mouse cursor
    // ------------------------------------------------------------------
    Mouse  mousePos;
    AEVec2 mouseDir{ 0.0f, 0.0f };

    GetMouseWorldPosition(mousePos.position.x, mousePos.position.y);

    mouseDir.x = mousePos.position.x - transform.position.x;
    mouseDir.y = mousePos.position.y - transform.position.y;
    AEVec2Normalize(&mouseDir, &mouseDir);

    // ------------------------------------------------------------------
    // 4. Shooting - continuous fire on left mouse hold (10 shots/sec)
    // ------------------------------------------------------------------
    shootCooldown -= deltaTime;

    if (AEInputCheckCurr(AEVK_LBUTTON) && shootCooldown <= 0.0f)
    {
        Shoot(mouseDir);
        shootCooldown = 0.10f; // 10 shots per second
        if (shootSFX) shootSFX->Play();

        // Emit a small smoke puff at the player's position
        smokePS.Emit(transform.position);
    }
    smokePS.Update(deltaTime);   // Always tick, even when not shooting

    // ------------------------------------------------------------------
    // 5. NPC interactions: AoE damage ring + contact knockback/damage
    // ------------------------------------------------------------------
    const f32 aoeRadius = GetAoeRadius();
    const f32 aoeDamage = GetAoeDamage();

    for (GameObject* n : gamePageObj)
    {
        if (n->ObjectType != NP) continue;

        NPC* np = dynamic_cast<NPC*>(n);
        if (!np) continue;

        // Direction and distance from player to NPC
        const AEVec2 toNPC = {
            np->transform.position.x - transform.position.x,
            np->transform.position.y - transform.position.y
        };
        const f32 dist = sqrtf(toNPC.x * toNPC.x + toNPC.y * toNPC.y);

        // --- AoE damage ring (shows red tint while in range) ---
        if (dist < aoeRadius)
        {
            np->health -= aoeDamage * deltaTime;
            np->spriteRenderer.colour = { 1.0f, 0.0f, 0.0f, 1.0f };
        }
        else
        {
            np->spriteRenderer.colour = np->baseColour;
        }

        // --- Contact collision damage + knockback ---
        if (!np->isActive) continue;

        const f32 playerRadius = (transform.scale.x + transform.scale.y) / 4.0f;
        const f32 npcRadius = (np->transform.scale.x + np->transform.scale.y) / 4.0f;
        const f32 collisionThresh = playerRadius + npcRadius;

        if (dist < collisionThresh)
        {
            // Damage and knockback vary by NPC type
            f32 contactDamage = 0.0f;
            f32 knockbackForce = 0.0f;

            switch (np->type)
            {
            case NPC_MELEE:
                contactDamage = 50.0f * deltaTime;   // High damage
                knockbackForce = 300.0f;
                break;

            case NPC_BOSS:
                contactDamage = 100.0f * deltaTime;  // Massive damage
                knockbackForce = 500.0f;

                // Log boss contact at most every 0.2s to avoid spam
                {
                    static f32 flashTimer = 0.0f;
                    flashTimer += deltaTime;
                    if (flashTimer > 0.2f)
                    {
                        std::cout << "[BOSS] Contact! 100 dmg/sec\n";
                        flashTimer = 0.0f;
                    }
                }
                break;

            case NPC_WALK:
            case NPC_RANGER:
                contactDamage = 20.0f * deltaTime;  // Light damage
                knockbackForce = 200.0f;
                break;
            }

            // Apply damage (clamped to 0)
            health -= contactDamage;
            if (health < 0.0f) health = 0.0f;

            // Apply knockback: push player away from the NPC
            if (dist > 0.0f && knockbackForce > 0.0f)
            {
                transform.position.x -= (toNPC.x / dist) * knockbackForce * deltaTime;
                transform.position.y -= (toNPC.y / dist) * knockbackForce * deltaTime;
            }
        }
    }

    // ------------------------------------------------------------------
    // 6. World boundary clamp (applied last, after all movement)
    // ------------------------------------------------------------------
    const f32 halfW = WORLD_WIDTH / 2.0f;
    const f32 halfH = WORLD_HEIGHT / 2.0f;
    const f32 halfPx = transform.scale.x / 2.0f;
    const f32 halfPy = transform.scale.y / 2.0f;

    if (transform.position.x > halfW - halfPx) transform.position.x = halfW - halfPx;
    if (transform.position.x < -halfW + halfPx) transform.position.x = -halfW + halfPx;
    if (transform.position.y > halfH - halfPy) transform.position.y = halfH - halfPy;
    if (transform.position.y < -halfH + halfPy) transform.position.y = -halfH + halfPy;
}

// ============================================================================
// Shoot
// ============================================================================
// Finds the first inactive player-owned bullet in the pool and fires it
// in the given direction. If no bullets are available, does nothing.
// ============================================================================
void Player::Shoot(AEVec2 dir)
{
    for (GameObject* obj : gamePageObj)
    {
        if (obj->ObjectType != SHOT || obj->isActive) continue;

        Bullet* bullet = dynamic_cast<Bullet*>(obj);
        if (!bullet || bullet->owner != BulletOwner::PLAYER) continue;

        // Reactivate this pooled bullet
        bullet->isActive = true;
        bullet->transform.position = transform.position;
        bullet->dir = dir;
        bullet->lifeTime = bullet->maxLifeTime;
        bullet->spriteRenderer.colour = { 1.0f, 1.0f, 0.0f, 1.0f }; // Yellow
        break; // One bullet per Shoot() call
    }
}

// ============================================================================
// Power-Up Getters
// ============================================================================
// These delegate to the PowerUpSystem so all stat scaling goes through
// one place. If no PowerUpSystem is assigned, safe defaults are returned.
// ============================================================================

f32 Player::GetSpeed() const
{
    return powerUpSystem ? powerUpSystem->GetStats().GetTotalSpeed() : 2000.0f;
}

f32 Player::GetBulletDamage() const
{
    return powerUpSystem ? powerUpSystem->GetStats().GetTotalBulletDamage() : 100.0f;
}

f32 Player::GetAoeDamage() const
{
    return powerUpSystem ? powerUpSystem->GetStats().GetTotalAoeDamage() : 100.0f;
}

f32 Player::GetAoeRadius() const
{
    return powerUpSystem ? powerUpSystem->GetStats().GetTotalAoeRadius() : 100.0f;
}
