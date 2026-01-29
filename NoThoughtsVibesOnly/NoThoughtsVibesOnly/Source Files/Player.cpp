#include <iostream>
#include "Player.h"
#include "GameObject.h"

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

    std::cout << "Player - Update: Player Position (" << this->transform.position.x << ',' << this->transform.position.y << ")\n";
}