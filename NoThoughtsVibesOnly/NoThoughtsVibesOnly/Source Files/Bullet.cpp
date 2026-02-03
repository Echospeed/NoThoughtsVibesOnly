#include "pch.h"
#include "Bullet.h"
#include "GamePage.h"
#include "NPC.h"
#include "Player.h"
#include "GameObjectType.h"
#include <math.h>

extern GameObject* pPlayer;

void Bullet::Start()
{
    if (!startPos) return;

    transform.position = startPos->transform.position;
    transform.scale = { 10.0f, 10.0f };
    transform.rotation = 0.0f;

    spriteRenderer.width = 0.0f;
    spriteRenderer.height = 0.0f;
    spriteRenderer.texture = nullptr;
    spriteRenderer.colour = { 1.0f, 1.0f, 0.0f, 1.0f };
    spriteRenderer.meshType = MESH_CIRCLE;

    ObjectType = ObjectType::SHOT;
    isActive = true;
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

    // --- NEW FIX: Deactivate if off-screen ---
    // Assuming WORLD_WIDTH/HEIGHT are accessible (you might need to include GamePage.h or use extern)
    // Or just use a large safe number like 2000.0f
    if (transform.position.x > 2000.0f || transform.position.x < -2000.0f ||
        transform.position.y > 2000.0f || transform.position.y < -2000.0f)
    {
        HideBullet();
        return;
    }

    if (owner == BulletOwner::PLAYER)
    {
        // Interact with NPCs
        for (auto& obj : objects)
        {
            if (!obj || !obj->isActive || obj->ObjectType != NP) continue;

            NPC* npc = dynamic_cast<NPC*>(obj);
            if (!npc || !npc->isVisibleToPlayer) continue;

            AEVec2 toNPC = { npc->transform.position.x - transform.position.x,
                             npc->transform.position.y - transform.position.y };
            f32 dist = sqrtf(toNPC.x * toNPC.x + toNPC.y * toNPC.y);

            if (dist < 30.0f)
            {
				npc->health -= 100.0f; // player bullet does 5 damage
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
            // TODO: reduce player health
			Player* player = dynamic_cast<Player*>(pPlayer);
			player->health -= 25.0f; //enemy bullet does 5 damage
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
    // Set color based on owner
        if (owner == BulletOwner::PLAYER)
            spriteRenderer.colour = { 1.0f, 1.0f, 0.0f, 1.0f }; // yellow
        else
            spriteRenderer.colour = { 1.0f, 0.0f, 0.0f, 1.0f }; // red for enemy
}
