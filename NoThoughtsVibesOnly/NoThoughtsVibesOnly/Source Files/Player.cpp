#include "pch.h"
#include "Player.h"
#include "AEEngine.h"
#include <iostream>
#include "Bullet.h"
#include "NPC.h"
#include "GamePage.h"
#include <math.h>
#include "Input.h"

void Player::Start()
{
    transform.position = { 0.0f, 0.0f };
    transform.scale = { 50.0f, 50.0f };
    transform.rotation = 0.0f;

    spriteRenderer.colour.r = 1.0f;
    spriteRenderer.colour.g = 0.0f;
    spriteRenderer.colour.b = 0.0f;

    //std::cout << "Player Initialized at ("        << transform.position.x << ", " << transform.position.y << ")\n";
}

void Player::Update(f32 deltaTime)
{
    // ------------------------
    // Movement
    // ------------------------
    if (AEInputCheckCurr(AEVK_W)) transform.position.y += 300.0f * deltaTime;
    if (AEInputCheckCurr(AEVK_S)) transform.position.y -= 300.0f * deltaTime;
    if (AEInputCheckCurr(AEVK_A)) transform.position.x -= 300.0f * deltaTime;
    if (AEInputCheckCurr(AEVK_D)) transform.position.x += 300.0f * deltaTime;

    // ------------------------
    // Rotation via QE
    // ------------------------
    if (AEInputCheckCurr(AEVK_Q)) transform.rotation -= rotationSpeed * deltaTime;
    if (AEInputCheckCurr(AEVK_E)) transform.rotation += rotationSpeed * deltaTime;

    // ------------------------
    // Shooting
    // ------------------------
        
    Mouse mousePos = { 0.0f, 0.0f };
    AEVec2 dir = { 0.0f, 0.0f };

    //mouse.x and mouse.y are in screen coordinates, need to convert to world coordinates
    GetMouseWorldPosition(mousePos.position.x, mousePos.position.y);
    dir.x = mousePos.position.x - transform.position.x;
    dir.y = mousePos.position.y - transform.position.y;

    AEVec2Normalize(&dir, &dir);

	//std::cout << "Player Position (" << transform.position.x << ", " << transform.position.y << ")\n";
	//std::cout << "Mouse Position (" << mousePos.position.x << ", " << mousePos.position.y << ")\n";
	//std::cout << "Player Shooting towards (" << dir.x << ", " << dir.y << ")\n";

    shootCooldown -= deltaTime;
    if (AEInputCheckTriggered(AEVK_LBUTTON)) //&& shootCooldown <= 0.0f
    {
        Shoot(dir);
        shootCooldown = 0.20f; // fire rate 4/sec
    }

    // ------------------------
    // NPC Interaction (unchanged)
    // ------------------------
    for (GameObject* n : objects)
    {
        if (n->ObjectType != NP) continue;

        AEVec2 dir {0.0f, 0.0f};
        dir.x = n->transform.position.x - transform.position.x;
        dir.y = n->transform.position.y - transform.position.y;
        f32 dist = sqrtf(dir.x * dir.x + dir.y * dir.y);

        NPC* np = dynamic_cast<NPC*>(n);
        if (np)
        {
            if (dist < 100.0f)
            {
                np->health -= 5.0f * deltaTime;
                np->spriteRenderer.colour.r = 1.0f;
                np->spriteRenderer.colour.g = 0.0f;
                np->spriteRenderer.colour.b = 0.0f;
            }
            else
            {
                np->spriteRenderer.colour = np->baseColour;
            }
        }
    }
}

void Player::Shoot(AEVec2 dir)
{
    // Create Bullet
    for(GameObject* obj : objects)
    {
        if (obj->ObjectType == SHOT && !obj->isActive)
        {
            Bullet* bullet = dynamic_cast<Bullet*>(obj);
            if (bullet->owner == BulletOwner::PLAYER)
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