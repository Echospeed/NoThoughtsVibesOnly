#include "Enemy.h"
#include <iostream>
const float WORLD_WIDTH = 2000.0f;
const float WORLD_HEIGHT = 2000.0f;

namespace {

	AEVec2 operator-(const AEVec2& a, const AEVec2& b) {
		return { a.x - b.x, a.y - b.y };
	}

	AEVec2 operator*(const AEVec2& a, f32 scalar) {
		return { a.x * scalar, a.y * scalar };
	}

	AEVec2 ScaleVector(const AEVec2& vec, float scalar) {
		return { vec.x * scalar, vec.y * scalar };
	}

	AEVec2 AddVectors(const AEVec2& a, const AEVec2& b) {
		return { a.x + b.x, a.y + b.y };
	}

}

Enemy::Enemy(Player* target)
{
	targetPlayer = target;

	if (!targetPlayer)
		std::cout << "Warning: AIEnemy has a null player reference!\n";

	// DO NOT push into objects manually! It's already handled.
	spriteRenderer.texture = nullptr; // we just want a colored shape

	// Initialize velocity to 0
	velX = 0.0f;
	velY = 0.0f;
}

void Enemy::Start() //Initialize enemy properties
{
	float rX = (rand() % static_cast<int>(2000.0f - (2000.0f/ 2.0f)));
	float rY = (rand() % static_cast<int>(2000.0f - (2000.0f / 2.0f)));

	transform.position = { rX, rY};
	transform.scale = { 30.0f, 30.0f };
	transform.rotation = 0.0f;
	spriteRenderer.colour.r = 1.0f;
	spriteRenderer.colour.g = 1.0f;
	spriteRenderer.colour.b = 0.0f;
	spriteRenderer.meshType = MESH_SQUARE;
	std::cout << "Enemy - Start: Enemy Initialized at Position (" << this->transform.position.x << ',' << this->transform.position.y << ")\n";
}

void Enemy::Update(f32 deltaTime) //Update enemy each frame
{

	//f32 speed = 3.0f;
	// Simple enemy logic can be added here

	if (!targetPlayer) return; // safety check

	// basically moves towards your player the further it is from the player the faster it moves
	AEVec2 displacement = targetPlayer->transform.position - this->transform.position;
	AEVec2 damped_displacement = ScaleVector(displacement, 0.02f); // Dampen the movement
	this->transform.position = AddVectors(this->transform.position, damped_displacement);
	std::cout << "Enemy - Update: Enemy Position (" << this->transform.position.x << ',' << this->transform.position.y << ")\n";
}