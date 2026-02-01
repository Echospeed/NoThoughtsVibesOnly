#include <iostream>
#include "Player.h"
#include "NPC.h"
#include "GameObject.h"
#include "GamePage.h"
#include <Math.h>

void Player::Start() //Initialize player properties
{
    transform.position = { 0.0f, 0.0f };
    transform.scale = { 50.0f, 50.0f };
    transform.rotation = 0.0f;

    spriteRenderer.colour.r = 1.0f;
    spriteRenderer.colour.g = 0.0f;
    spriteRenderer.colour.b = 0.0f;
	std::cout << "Player - Start: Player Initialized at Position (" << this->transform.position.x << ',' << this->transform.position.y << ")\n";
}

void Player::Update(f32 deltaTime) //Update player each frame
{
    if (AEInputCheckCurr(AEVK_W)) this->transform.position.y += 300.0f * deltaTime;
    if (AEInputCheckCurr(AEVK_S)) this->transform.position.y -= 300.0f * deltaTime;
    if (AEInputCheckCurr(AEVK_A)) this->transform.position.x -= 300.0f * deltaTime;
    if (AEInputCheckCurr(AEVK_D)) this->transform.position.x += 300.0f * deltaTime;

    for (GameObject* n : objects)
    {
        if (n->ObjectType != NP)
        {
            continue;
        }

        // Check if its within player range
        AEVec2 dir;
        dir.x = n->transform.position.x - this->transform.position.x;
        dir.y = n->transform.position.y - this->transform.position.y;

        f32 dist = sqrt((dir.x * dir.x) + (dir.y * dir.y));

        NPC* np = dynamic_cast<NPC*>(n);
        if (np != nullptr)
        {
            if (dist < 100.0f)
            {
                np->health -= 5.0f * deltaTime;
                np->spriteRenderer.colour.r = 1.0f;
                np->spriteRenderer.colour.g = 0.0f;
                np->spriteRenderer.colour.b = 0.0f;
                std::cout << "Player - Update: The NPC health is currently at " << np->health << std::endl;
            }
            else
            {
                np->spriteRenderer.colour.r = np->baseColour.r;
		        np->spriteRenderer.colour.g = np->baseColour.g;
		        np->spriteRenderer.colour.b = np->baseColour.b;
            }
        }
    }
    std::cout << "Player - Update: Player Position (" << this->transform.position.x << ',' << this->transform.position.y << ")\n";
}