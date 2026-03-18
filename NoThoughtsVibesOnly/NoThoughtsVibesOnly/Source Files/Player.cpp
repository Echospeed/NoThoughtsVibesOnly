// ============================================================================
// Player.cpp - Player Controller Implementation
// ============================================================================
// See Player.hpp for class declaration and usage.
//
// CONTROLS:
// ----------------------------------------------------------------------------
//   W / A / S / D              : Move
//   Left Arrow / Right Arrow   : Rotate
//   Left Mouse Hold            : Shoot toward cursor (10 shots/sec)
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


// ============================================================================
// Start
// ============================================================================
// Initialises the player at the world center with default stats.
// Called once from Game_Init() via obj->Start().
// ============================================================================
void Player::Start()
{
    const auto& pc = GameConfig::Player();

    transform.position = { 0.0f, 0.0f };
    transform.scale = { pc.scaleX, pc.scaleY };
    transform.rotation = 0.0f;

    spriteRenderer.colour = { 1.0f, 0.0f, 0.0f, 1.0f };

    shootCooldown = pc.shootSuppressCooldown;
    reloadDuration = pc.reloadDuration;
    isReloading = false;
    ammoInMagazine = powerUpSystem ? (int)powerUpSystem->GetStats().bulletCount
        : GameConfig::Player().startingAmmo;

    smokePS = ParticleSystem::MakeSmoke();

    playerSpritesheet = AEGfxTextureLoad(pc.texture.c_str());
    spriteRenderer.texture = playerSpritesheet;

    idleAnim = Animation(0.2f, true);

    //idleAnim.AddFrames(0, 0, 500, 500);
    //idleAnim.AddFrames(500, 0, 500, 500);
    //idleAnim.AddFrames(1000, 0, 500, 500);
    //idleAnim.AddFrames(1500, 0, 500, 500);

    //runAnim = Animation(0.1f, true);

    //runAnim.AddFrames(0, 1000, 500, 500);
    //runAnim.AddFrames(500, 1000, 500, 500);
    //runAnim.AddFrames(1000, 1000, 500, 500);
    //runAnim.AddFrames(1500, 1000, 500, 500);

    //playerAnimator.Play(&idleAnim);
}

// ============================================================================
// Update
// ============================================================================
// Runs every frame. Order of operations:
//   1. Read movement input (WASD) using current (upgraded) speed
//   2. Read rotation input (Left Right arrows)
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

    bool isMoving = false;

    if (AEInputCheckCurr(AEVK_W))
    {
        transform.position.y += currentSpeed * deltaTime;
        isMoving = true;
    }
    if (AEInputCheckCurr(AEVK_S))
    {
        transform.position.y -= currentSpeed * deltaTime;
        isMoving = true;
    }
    if (AEInputCheckCurr(AEVK_A))
    {
        transform.position.x -= currentSpeed * deltaTime;
        isMoving = true;
    }
    if (AEInputCheckCurr(AEVK_D))
    {
        transform.position.x += currentSpeed * deltaTime;
        isMoving = true;
    }

    // 1. The State Machine: Decide which animation to play
    //if (isMoving)
    //{
    //    playerAnimator.Play(&runAnim);
    //}
    //else
    //{
    //    playerAnimator.Play(&idleAnim);
    //}

    // ------------------------------------------------------------------
    // 2. Rotation
    // ------------------------------------------------------------------
    if (AEInputCheckCurr(AEVK_LEFT)) transform.rotation -= rotationSpeed * deltaTime;
    if (AEInputCheckCurr(AEVK_RIGHT)) transform.rotation += rotationSpeed * deltaTime;

    // ------------------------------------------------------------------
    // 3. Aim direction - from player toward mouse cursor
    // ------------------------------------------------------------------
    Mouse  mousePos{};
    AEVec2 mouseDir{ 0.0f, 0.0f };

    GetMouseWorldPosition(mousePos.position.x, mousePos.position.y);

    mouseDir.x = mousePos.position.x - transform.position.x;
    mouseDir.y = mousePos.position.y - transform.position.y;
    AEVec2Normalize(&mouseDir, &mouseDir);

    // ------------------------------------------------------------------
    // 4. Shooting - continuous fire on left mouse hold (10 shots/sec)
    // ------------------------------------------------------------------

    // --- Reload tick ---
    if (isReloading)
    {
        reloadTimer -= deltaTime;
        if (reloadTimer <= 0.0f)
        {
            // Reload complete: reset all player bullets to inactive and unspent
            for (GameObject* obj : gamePageObj)
            {
                if (obj->ObjectType != SHOT) continue;
                Bullet* b = dynamic_cast<Bullet*>(obj);
                if (!b || b->owner != BulletOwner::PLAYER) continue;
                b->isActive = false;
                b->spent = false;
                b->spriteRenderer.colour.a = 0.0f;
                b->transform.position = { -1000.0f, -1000.0f };
            }
            // Restore magazine count from the power-up system
            ammoInMagazine = powerUpSystem ? (int)powerUpSystem->GetStats().bulletCount
                : GameConfig::Player().startingAmmo;
            isReloading = false;
        }
    }

    // --- Manual reload on R key ---
    if (AEInputCheckTriggered(AEVK_R))
        TriggerReload();

    shootCooldown -= deltaTime;

    // Shooting is suppressed while the power-up overlay is open, and for one
    // extra frame after closing it so the selecting click doesn't also fire a bullet.
    // If a power-up was just picked by mouse click, skip shooting this frame,
    // reset the cooldown to add a small delay, and clear the flag.
    if (suppressShootOneFrame)
    {
        suppressShootOneFrame = false;
        shootCooldown = GameConfig::Player().shootSuppressCooldown;
    }
    else if (!isReloading && AEInputCheckCurr(AEVK_LBUTTON) && shootCooldown <= 0.0f
        && !(powerUpSystem && powerUpSystem->IsWaitingForUpgrade()))
    {
        Shoot(mouseDir);
        shootCooldown = GameConfig::Player().shootCooldown;
        AudioManager::PlaySFX("Shoot");

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

        const f32 npcRadius = (np->transform.scale.x + np->transform.scale.y) / 4.0f;

        // AoE damage ring - pulse the NPC dim/bright to show it's being damaged
        if (dist < (aoeRadius + npcRadius))
        {
            np->health -= aoeDamage * deltaTime;

            // Pulse alpha between 0.4 and 1.0 at ~4 Hz so the NPC visibly flickers,
            // indicating it is inside the AoE and taking damage.
            const f32 pulse = 0.7f + 0.3f * sinf((f32)AEGetTime(nullptr) * 25.0f);
            np->spriteRenderer.colour = {
                np->baseColour.r,
                np->baseColour.g * 0.3f, // drain green/blue to tint red
                np->baseColour.b * 0.3f,
                pulse                     // flickering alpha
            };
        }
        else
        {
            np->spriteRenderer.colour = np->baseColour;
        }

        // Contact collision damage + knockback
        if (!np->isActive) continue;

        const f32 playerRadius = (transform.scale.x + transform.scale.y) / 4.0f;
        const f32 collisionThresh = playerRadius + npcRadius;

        if (dist < collisionThresh)
        {
            // Damage and knockback vary by NPC type
            f32 contactDamage = 0.0f;
            f32 knockbackForce = 0.0f;

            const auto& wc = GameConfig::Wave();
            switch (np->type)
            {
            case NPC_MELEE:
                contactDamage = wc.contactDamage.melee * deltaTime;
                knockbackForce = wc.knockback.melee;
                break;
            case NPC_BOSS:
                contactDamage = wc.contactDamage.boss * deltaTime;
                knockbackForce = wc.knockback.boss;
                {
                    static f32 flashTimer = 0.0f;
                    flashTimer += deltaTime;
                    if (flashTimer > 0.2f) { std::cout << "[BOSS] Contact! dmg/sec\n"; flashTimer = 0.0f; }
                }
                break;
            case NPC_WALK:
            case NPC_RANGER:
                contactDamage = wc.contactDamage.walkRanger * deltaTime;
                knockbackForce = wc.knockback.walkRanger;
                break;
            }

            // Apply damage (clamped to 0) - ONLY IF NOT INVULNERABLE
            if (!invulnAbility.IsActive())
            {
                health -= contactDamage;
                if (health < 0.0f) health = 0.0f;
            }

            // Apply knockback: push player away from the NPC
            if (dist > 0.0f && knockbackForce > 0.0f)
            {
                transform.position.x -= (toNPC.x / dist) * knockbackForce * deltaTime;
                transform.position.y -= (toNPC.y / dist) * knockbackForce * deltaTime;
            }
        }
    }

    // ------------------------------------------------------------------
    // Update Abilities & Check Input
    // ------------------------------------------------------------------
    invulnAbility.Update(deltaTime);

    if (AEInputCheckTriggered(AEVK_E))
    {
        invulnAbility.TryActivate();
    }

    // Optional: Turn the player gold while invulnerable
    if (invulnAbility.IsActive()) {
        spriteRenderer.colour = { 1.0f, 0.8f, 0.0f, 1.0f };
    }
    else {
        spriteRenderer.colour = { 1.0f, 0.0f, 0.0f, 1.0f };
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


    // ------------------------------------------------------------------
    // 7. Animation
    // ------------------------------------------------------------------

    Rect currentFrame = playerAnimator.GetCurrentFrameRect();

    spriteRenderer.uOffset = (f32)currentFrame.x / 2000.0f;
    spriteRenderer.vOffset = (f32)currentFrame.y / 2500.0f;

    playerAnimator.Update(deltaTime);
}

// ============================================================================
// Shoot
// ============================================================================
// Finds the first inactive player-owned bullet in the pool and fires it
// in the given direction. If no bullets are available, does nothing.
// ============================================================================
void Player::Shoot(AEVec2 dir)
{
    if (isReloading || ammoInMagazine <= 0) return;

    // Find and fire one unspent player bullet
    for (GameObject* obj : gamePageObj)
    {
        if (obj->ObjectType != SHOT) continue;
        Bullet* bullet = dynamic_cast<Bullet*>(obj);
        if (!bullet || bullet->owner != BulletOwner::PLAYER) continue;
        if (bullet->isActive || bullet->spent) continue;

        bullet->spent = true;
        bullet->isActive = true;
        bullet->transform.position = transform.position;
        bullet->dir = dir;
        bullet->lifeTime = bullet->maxLifeTime;
        bullet->spriteRenderer.colour = { 1.0f, 1.0f, 0.0f, 1.0f };
        --ammoInMagazine;
        break;
    }

    // Magazine empty - start reload
    if (ammoInMagazine <= 0)
    {
        isReloading = true;
        reloadTimer = reloadDuration;
    }
}

// ============================================================================
// TriggerReload
// ============================================================================
// Manually starts a reload. No-ops if already reloading or magazine is full.
// ============================================================================
void Player::TriggerReload()
{
    const int maxAmmo = powerUpSystem ? (int)powerUpSystem->GetStats().bulletCount
        : GameConfig::Player().startingAmmo;
    if (isReloading || ammoInMagazine == maxAmmo) return;
    isReloading = true;
    reloadTimer = reloadDuration;
}

// ============================================================================
// Power-Up Getters
// ============================================================================
// These delegate to the PowerUpSystem so all stat scaling goes through
// one place. If no PowerUpSystem is assigned, safe defaults are returned.
// ============================================================================

f32 Player::GetSpeed() const
{
    return powerUpSystem ? powerUpSystem->GetStats().GetTotalSpeed()
        : GameConfig::Player().baseSpeed;
}

f32 Player::GetBulletDamage() const
{
    return powerUpSystem ? powerUpSystem->GetStats().GetTotalBulletDamage()
        : GameConfig::PowerUp().baseBulletDamage;
}

f32 Player::GetAoeDamage() const
{
    return powerUpSystem ? powerUpSystem->GetStats().GetTotalAoeDamage()
        : GameConfig::PowerUp().baseAoeDamage;
}

f32 Player::GetAoeRadius() const
{
    return powerUpSystem ? powerUpSystem->GetStats().GetTotalAoeRadius()
        : GameConfig::PowerUp().baseAoeRadius;
}

void Player::Free()
{
    if (playerSpritesheet != nullptr)
    {
        AEGfxTextureUnload(playerSpritesheet);
        playerSpritesheet = nullptr;
    }
}