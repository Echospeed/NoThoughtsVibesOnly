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

void Player::Start()
{
    transform.position = { 0.0f, 0.0f };
    transform.scale = { 50.0f, 50.0f };
    transform.rotation = 0.0f;

    spriteRenderer.colour.r = 1.0f;
    spriteRenderer.colour.g = 0.0f;
    spriteRenderer.colour.b = 0.0f;

    shootCooldown = 0.3f; // 0.3 second grace period

    //std::cout << "Player Initialized at ("        << transform.position.x << ", " << transform.position.y << ")\n";
}

void Player::Update(f32 deltaTime)
{
    // ========================
    // Get current speed from power-up system
    // ========================
    f32 currentSpeed = GetSpeed();
    
    // ------------------------
    // Movement
    // ------------------------
    if (AEInputCheckCurr(AEVK_W)) transform.position.y += currentSpeed * deltaTime;
    if (AEInputCheckCurr(AEVK_S)) transform.position.y -= currentSpeed * deltaTime;
    if (AEInputCheckCurr(AEVK_A)) transform.position.x -= currentSpeed * deltaTime;
    if (AEInputCheckCurr(AEVK_D)) transform.position.x += currentSpeed * deltaTime;

    // ------------------------
    // Rotation via QE
    // ------------------------
    if (AEInputCheckCurr(AEVK_Q)) transform.rotation -= rotationSpeed * deltaTime;
    if (AEInputCheckCurr(AEVK_E)) transform.rotation += rotationSpeed * deltaTime;

    // ------------------------
    // Shooting - CONTINUOUS ON LEFT CLICK HOLD
    // ------------------------

    Mouse mousePos = { 0.0f, 0.0f };
    AEVec2 mouseDir = { 0.0f, 0.0f };

    //mouse.x and mouse.y are in screen coordinates, need to convert to world coordinates
    GetMouseWorldPosition(mousePos.position.x, mousePos.position.y);
    mouseDir.x = mousePos.position.x - transform.position.x;
    mouseDir.y = mousePos.position.y - transform.position.y;

    AEVec2Normalize(&mouseDir, &mouseDir);

    // Decrease cooldown timer
    shootCooldown -= deltaTime;

    // Continuous shooting on LEFT CLICK HOLD
    if (AEInputCheckCurr(AEVK_LBUTTON) && shootCooldown <= 0.0f)
    {
        Shoot(mouseDir);
        shootCooldown = 0.10f; // fire rate 10/sec (faster for continuous shooting)
    }

    // ========================
    // Get upgraded AoE values
    // ========================
    f32 aoeRadius = GetAoeRadius();
    f32 aoeDamage = GetAoeDamage();
    
    // ------------------------
    // NPC Interaction - AoE Damage & Collision
    // ------------------------
    for (GameObject* n : gamePageObj)
    {
        if (n->ObjectType != NP) continue;

        AEVec2 dir{ 0.0f, 0.0f };
        dir.x = n->transform.position.x - transform.position.x;
        dir.y = n->transform.position.y - transform.position.y;
        f32 dist = sqrtf(dir.x * dir.x + dir.y * dir.y);

        NPC* np = dynamic_cast<NPC*>(n);
        if (np)
        {
            // Player's AoE damage to NPCs (using upgraded values)
            if (dist < aoeRadius)
            {
                np->health -= aoeDamage * deltaTime; // Use upgraded damage
                np->spriteRenderer.colour.r = 1.0f;
                np->spriteRenderer.colour.g = 0.0f;
                np->spriteRenderer.colour.b = 0.0f;
            }
            else
            {
                np->spriteRenderer.colour = np->baseColour;
            }

            // NPC collision damage to player
            // Calculate collision distance based on both sizes
            f32 playerRadius = (transform.scale.x + transform.scale.y) / 4.0f; // Average radius
            f32 npcRadius = (np->transform.scale.x + np->transform.scale.y) / 4.0f;
            f32 collisionDist = playerRadius + npcRadius;

            if (dist < collisionDist && np->isActive)
            {
                // Different damage based on NPC type
                f32 contactDamage = 0.0f;
                f32 knockbackForce = 0.0f;

                if (np->type == NPC_MELEE)
                {
                    contactDamage = 50.0f * deltaTime; // Bomber does high contact damage
                    knockbackForce = 300.0f;
                }
                else if (np->type == NPC_BOSS)
                {
                    contactDamage = 100.0f * deltaTime; // Boss does MASSIVE contact damage!
                    knockbackForce = 500.0f; // Heavy knockback!

                    // Flash warning when hit by boss
                    static f32 flashTimer = 0.0f;
                    flashTimer += deltaTime;
                    if (flashTimer > 0.2f) // Print warning every 0.2 seconds
                    {
                        std::cout << "💥 BOSS COLLISION! Taking " << (100.0f) << " damage/sec!\n";
                        flashTimer = 0.0f;
                    }
                }
                else if (np->type == NPC_WALK || np->type == NPC_RANGER)
                {
                    contactDamage = 20.0f * deltaTime; // Walker/Ranger do light contact damage
                    knockbackForce = 200.0f;
                }

                // Apply damage
                health -= contactDamage;

                if (health <= 0.0f)
                {
                    health = 0.0f;
                }

                // Apply knockback - push player away from NPC
                if (dist > 0.0f && knockbackForce > 0.0f)
                {
                    dir.x /= dist; // Normalize
                    dir.y /= dist;

                    // Push player away
                    transform.position.x -= dir.x * knockbackForce * deltaTime;
                    transform.position.y -= dir.y * knockbackForce * deltaTime;
                }
            }
        }
    }

    // ========================
    // ENFORCE WORLD BOUNDARIES (FINAL - After all movement/collision)
    // ========================
    f32 halfWorldWidth = WORLD_WIDTH / 2.0f;
    f32 halfWorldHeight = WORLD_HEIGHT / 2.0f;
    f32 halfPlayerWidth = transform.scale.x / 2.0f;
    f32 halfPlayerHeight = transform.scale.y / 2.0f;

    // Clamp player position to world bounds
    if (transform.position.x > halfWorldWidth - halfPlayerWidth)
        transform.position.x = halfWorldWidth - halfPlayerWidth;
    if (transform.position.x < -halfWorldWidth + halfPlayerWidth)
        transform.position.x = -halfWorldWidth + halfPlayerWidth;
    if (transform.position.y > halfWorldHeight - halfPlayerHeight)
        transform.position.y = halfWorldHeight - halfPlayerHeight;
    if (transform.position.y < -halfWorldHeight + halfPlayerHeight)
        transform.position.y = -halfWorldHeight + halfPlayerHeight;
}

void Player::Shoot(AEVec2 dir)
{
    // Create Bullet
    for (GameObject* obj : gamePageObj)
    {
        if (obj->ObjectType == SHOT && !obj->isActive)
        {
            Bullet* bullet = dynamic_cast<Bullet*>(obj);
            if (bullet && bullet->owner == BulletOwner::PLAYER)
            {
                bullet->isActive = true;
                bullet->transform.position = transform.position;
                bullet->dir = dir; // set direction
                bullet->lifeTime = bullet->maxLifeTime; // reset lifetime
                bullet->spriteRenderer.colour.a = 1.0f; // make visible
                break;
            }
        }
    }
}

// ============================================================================
// Power-Up System Getter Methods
// ============================================================================

f32 Player::GetSpeed() const
{
    if (powerUpSystem)
        return powerUpSystem->GetStats().GetTotalSpeed();
    return 2000.0f; // Default
}

f32 Player::GetBulletDamage() const
{
    if (powerUpSystem)
        return powerUpSystem->GetStats().GetTotalBulletDamage();
    return 100.0f; // Default
}

f32 Player::GetAoeDamage() const
{
    if (powerUpSystem)
        return powerUpSystem->GetStats().GetTotalAoeDamage();
    return 100.0f; // Default
}

f32 Player::GetAoeRadius() const
{
    if (powerUpSystem)
        return powerUpSystem->GetStats().GetTotalAoeRadius();
    return 100.0f; // Default
}
