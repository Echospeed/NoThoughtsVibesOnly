// Author: John Chiow

// ============================================================================
// Bullet.cpp - Projectile Object Implementation
// ============================================================================
// See Bullet.hpp for full documentation, pool setup, and firing examples.
// ============================================================================

#include "pch.hpp"
#include "Bullet.hpp"
#include "GamePage.hpp"
#include "NPC.hpp"
#include "Player.hpp"
#include "GameObjectType.hpp"
#include <math.h>

// Global player pointer - declared in GamePage.cpp, used for enemy hit detection
extern GameObject* pPlayer;

// ============================================================================
// Start
// ============================================================================
// Initialises bullet state after construction.
// Pooled bullets start with startPos == nullptr and are positioned offscreen
// until they are assigned and fired via Activate() or manual fire logic.
// ============================================================================
void Bullet::Start()
{
    // Position at shooter if available, otherwise park offscreen
    transform.position = startPos ? startPos->transform.position
        : AEVec2{ -1000.0f, -1000.0f };

    transform.scale = { GameConfig::Gameplay().bullet.defaultScale,
                        GameConfig::Gameplay().bullet.defaultScale };
    transform.rotation = 0.0f;

    lifeTime = 0.0f;
    maxLifeTime = GameConfig::Gameplay().bullet.maxLifeTime;
    spriteRenderer.texture = nullptr;
    spriteRenderer.meshType = MESH_CIRCLE;

    ObjectType = ObjectType::SHOT;

    // Pooled bullets without an assigned shooter start hidden
    if (!startPos)
    {
        isActive = false;
        spriteRenderer.colour.a = 0.0f;
    }
}

// ============================================================================
// Update
// ============================================================================
// Per-frame bullet logic. Only runs if the bullet is active.
//
// Order of operations:
//   1. Move bullet along dir * speed
//   2. Count down lifetime - despawn if expired
//   3. Clamp to world boundaries - despawn if out of bounds
//   4. Check collision with target (NPC for player bullets, Player for enemy)
// ============================================================================
void Bullet::Update(f32 deltaTime)
{
    if (!isActive) return;

    // --- Movement ---
    transform.position.x += dir.x * speed * deltaTime;
    transform.position.y += dir.y * speed * deltaTime;

    // --- Lifetime countdown ---
    lifeTime -= deltaTime;
    if (lifeTime <= 0.0f)
    {
        HideBullet();
        return;
    }

    // --- World boundary check ---
    const f32 halfW = WORLD_WIDTH / 2.0f;
    const f32 halfH = WORLD_HEIGHT / 2.0f;
    if (transform.position.x > halfW || transform.position.x < -halfW ||
        transform.position.y >  halfH || transform.position.y < -halfH)
    {
        HideBullet();
        return;
    }

    // --- Collision detection ---
    if (owner == BulletOwner::PLAYER)
    {
        // Player bullets damage any visible, active NPC
        for (auto& obj : gamePageObj)
        {
            if (!obj || !obj->isActive || obj->ObjectType != NP) continue;

            NPC* npc = dynamic_cast<NPC*>(obj);
            if (!npc || !npc->isVisibleToPlayer) continue;

            const AEVec2 toNPC = {
                npc->transform.position.x - transform.position.x,
                npc->transform.position.y - transform.position.y
            };
            const f32 distSq = toNPC.x * toNPC.x + toNPC.y * toNPC.y;

            // Hit radius = half the NPC's average dimension (fits any size incl. boss)
            const f32 npcRadius = (npc->transform.scale.x + npc->transform.scale.y) * 0.25f;
            if (distSq < npcRadius * npcRadius)
            {
                // Apply upgraded damage from the firing player
                Player* player = dynamic_cast<Player*>(startPos);
                npc->health -= player ? player->GetBulletDamage() : 100.0f;
                // Smoke puff at impact point (bigger burst for boss)
                const int burstCount = (npc->type == NPC_BOSS) ? 25 : 12;
                g_EnemyHitPS.EmitBurst(transform.position, burstCount);
                HideBullet();
                break; // One bullet hits one NPC
            }
        }
    }
    else if (owner == BulletOwner::ENEMY)
    {
        if (!pPlayer) return;

        const AEVec2 toPlayer = {
            pPlayer->transform.position.x - transform.position.x,
            pPlayer->transform.position.y - transform.position.y
        };
        const f32 distSq = toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y;

        // Hit radius = half the player's average dimension
        const f32 playerRadius = (pPlayer->transform.scale.x + pPlayer->transform.scale.y) * 0.25f;
        if (distSq < playerRadius * playerRadius)
        {
            Player* player = dynamic_cast<Player*>(pPlayer);
            if (player)
            {
                // ONLY take damage if the invulnerability ability is NOT active
                if (!player->invulnAbility.IsActive())
                {
                    player->health -= GameConfig::Gameplay().bullet.enemyDamage;
                    // Red spark burst only when damage actually lands
                    g_PlayerHitPS.EmitBurst(transform.position, 10);
                }
            }

            // Bullet still gets destroyed even if blocked
            HideBullet();
        }
    }
}

// ============================================================================
// Activate
// ============================================================================
// Sets up and fires this bullet from a given shooter toward a direction.
// Use this as the clean way to fire a pooled bullet.
// ============================================================================
void Bullet::Activate(GameObject* shooter, AEVec2 direction, BulletOwner newOwner)
{
    startPos = shooter;
    dir = direction;
    owner = newOwner;
    isActive = true;
    lifeTime = maxLifeTime;
    spriteRenderer.colour.a = 1.0f;

    // Set speed based on owner
    if (owner == BulletOwner::PLAYER)
        speed = GameConfig::Gameplay().bullet.playerSpeed;
    else
        speed = GameConfig::Gameplay().bullet.enemySpeed;

    // Spawn at shooter's current position
    if (shooter) transform.position = shooter->transform.position;

    // Colour-code by owner for easy visual debugging
    spriteRenderer.colour = (owner == BulletOwner::PLAYER)
        ? Colour{ 1.0f, 1.0f, 0.0f, 1.0f }  // Yellow = player
    : Colour{ 1.0f, 0.0f, 0.0f, 1.0f }; // Red    = enemy
}

// ============================================================================
// HideBullet (private)
// ============================================================================
// Deactivates the bullet and moves it offscreen so it is invisible and
// available for reuse by the pool. Called internally on hit/expiry/bounds.
// ============================================================================
void Bullet::HideBullet()
{
    isActive = false;
    spriteRenderer.colour.a = 0.0f;
    transform.position = { -1000.0f, -1000.0f };
}