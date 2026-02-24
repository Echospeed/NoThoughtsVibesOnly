#include "pch.hpp"
#include "Bullet.hpp"
#include "GamePage.hpp"
#include "NPC.hpp"
#include "Player.hpp"
#include "GameObjectType.hpp"
#include <math.h>

extern GameObject* pPlayer;

void Bullet::Start()
{
    // FIX: Don't return early - initialize even if startPos is null
    // Pooled bullets start with null startPos and get assigned later
    if (startPos) {
        transform.position = startPos->transform.position;
    } else {
        transform.position = { -1000.0f, -1000.0f }; // offscreen
    }
    
    transform.scale = { 10.0f, 10.0f };
    transform.rotation = 0.0f;

    spriteRenderer.width = 0.0f;
    spriteRenderer.height = 0.0f;
    spriteRenderer.texture = nullptr;
    spriteRenderer.meshType = MESH_CIRCLE;

    ObjectType = ObjectType::SHOT;
    
    // Pooled bullets should start inactive
    // They will be activated when fired
    if (!startPos) {
        isActive = false;
        spriteRenderer.colour.a = 0.0f;
    }
}

void Bullet::Update(f32 deltaTime)
{
    if (!isActive) return;

    // Move bullet
    transform.position.x += dir.x * speed * deltaTime;
    transform.position.y += dir.y * speed * deltaTime;

    // Lifetime countdown
    lifeTime -= deltaTime;
    if (lifeTime <= 0.0f) 
    { 
        HideBullet(); 
        return; 
    }

    // Deactivate if outside world boundaries
    f32 halfWorldWidth = WORLD_WIDTH / 2.0f;
    f32 halfWorldHeight = WORLD_HEIGHT / 2.0f;
    
    if (transform.position.x > halfWorldWidth || transform.position.x < -halfWorldWidth ||
        transform.position.y > halfWorldHeight || transform.position.y < -halfWorldHeight)
    {
        HideBullet();
        return;
    }

    if (owner == BulletOwner::PLAYER)
    {
        // Interact with NPCs
        for (auto& obj : gamePageObj)
        {
            if (!obj || !obj->isActive || obj->ObjectType != NP) continue;

            NPC* npc = dynamic_cast<NPC*>(obj);
            if (!npc || !npc->isVisibleToPlayer) continue;

            AEVec2 toNPC = { npc->transform.position.x - transform.position.x,
                             npc->transform.position.y - transform.position.y };
            f32 dist = sqrtf(toNPC.x * toNPC.x + toNPC.y * toNPC.y);

            if (dist < 30.0f)
            {
                // Get upgraded damage from player
                Player* player = dynamic_cast<Player*>(startPos);
                f32 damage = player ? player->GetBulletDamage() : 100.0f;
                
                npc->health -= damage;
                HideBullet();
                break;
            }
        }
    }
    else if (owner == BulletOwner::ENEMY)
    {
        if (!pPlayer) return;

        AEVec2 toPlayer = { pPlayer->transform.position.x - transform.position.x,
                            pPlayer->transform.position.y - transform.position.y };
        f32 dist = sqrtf(toPlayer.x * toPlayer.x + toPlayer.y * toPlayer.y);

        if (dist < 30.0f)
        {
            Player* player = dynamic_cast<Player*>(pPlayer);
            player->health -= 25.0f; // enemy bullet does 25 damage
            HideBullet();
        }
    }
}

void Bullet::HideBullet()
{
    isActive = false;
    spriteRenderer.colour.a = 0.0f;
    transform.position = { -1000.0f, -1000.0f };
}

void Bullet::Activate(GameObject* shooter, AEVec2 direction, BulletOwner newOwner)
{
    startPos = shooter;
    dir = direction;
    owner = newOwner;
    isActive = true;
    lifeTime = maxLifeTime;
    spriteRenderer.colour.a = 1.0f;
    
    // Set position to shooter's position
    if (shooter) {
        transform.position = shooter->transform.position;
    }
    
    // Set color based on owner
    if (owner == BulletOwner::PLAYER)
        spriteRenderer.colour = { 1.0f, 1.0f, 0.0f, 1.0f }; // yellow
    else
        spriteRenderer.colour = { 1.0f, 0.0f, 0.0f, 1.0f }; // red for enemy
}
