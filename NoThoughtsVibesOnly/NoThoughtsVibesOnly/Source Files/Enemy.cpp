#include "Enemy.h"
#include <iostream>
void Enemy::Start() //Initialize enemy properties
{
	float rX = (rand() % static_cast<int>(2000.0f - (2000.0f/ 2.0f)));
	float rY = (rand() % static_cast<int>(2000.0f - (2000.0f / 2.0f)));

	transform.position = { rX, rY};
	transform.scale = { 1.0f, 1.0f };
	transform.rotation = 0.0f;
	spriteRenderer.colour.r = 1.0f;
	spriteRenderer.colour.g = 1.0f;
	spriteRenderer.colour.b = 0.0f;
	spriteRenderer.meshType = MESH_TRIANGLE;
	std::cout << "Enemy - Start: Enemy Initialized at Position (" << this->transform.position.x << ',' << this->transform.position.y << ")\n";
}

void Enemy::Update(f32 deltaTime) //Update enemy each frame
{
	// Simple enemy logic can be added here
	std::cout << "Enemy - Update: Enemy Position (" << this->transform.position.x << ',' << this->transform.position.y << ")\n";
}