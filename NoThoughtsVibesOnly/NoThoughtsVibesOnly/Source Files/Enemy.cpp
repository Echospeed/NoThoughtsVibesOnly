#include "Enemy.h"
#include <iostream>
const float WORLD_WIDTH = 2000.0f;
const float WORLD_HEIGHT = 2000.0f;

namespace {
	
	f32 speed = 50.0f;

	AEVec2 operator-(const AEVec2& a, const AEVec2& b) {
		return { a.x - b.x, a.y - b.y };
	}

	AEVec2 operator*(const AEVec2& a, f32 scalar) {
		return { a.x * scalar, a.y * scalar };
	}

	AEVec2 ScaleVector(const AEVec2& vec, float scalar) {
		return { vec.x * scalar, vec.y * scalar };
	}

	AEVec2 operator+(const AEVec2& a, const AEVec2& b) {
		return { a.x + b.x, a.y + b.y };
	}

}

Enemy::Enemy(Player* target)
{
	targetPlayer = target;

	if (!targetPlayer)
		std::cout << "Warning: AIEnemy has a null player reference!\n";

	// DO NOT push into objects manually! It's already handled.
	spriteRenderer.texture = nullptr; // We just want a colored shape

	// Initialize velocity to 0
	velX = 0.0f;
	velY = 0.0f;
}

void Enemy::Start() // Initialize enemy properties
{
	// Random starting position inside red box
	float halfWorldWidth = WORLD_WIDTH / 2.0f;
	float halfWorldHeight = WORLD_HEIGHT / 2.0f;

	transform.position = {
		(float)(rand() % (int)WORLD_WIDTH - halfWorldWidth),
		(float)(rand() % (int)WORLD_HEIGHT - halfWorldHeight)
	};

	transform.scale = { 15.0f, 15.0f }; // Enemy size
	transform.rotation = 0.0f;

	spriteRenderer.colour = { 1.0f, 1.0f, 0.0f }; // Yellow
	spriteRenderer.meshType = MESH_TRIANGLE;

	std::cout << "Enemy spawned at ("
		<< transform.position.x << ", "
		<< transform.position.y << ")\n";
}

void Enemy::Update(f32 deltaTime) // Update enemy each frame
{

	//f32 speed = 3.0f;
	// Simple enemy logic can be added here

	if (!targetPlayer) return; // Safety check

	float dx = targetPlayer->transform.position.x - transform.position.x;
	float dy = targetPlayer->transform.position.y - transform.position.y;
	float distance = std::sqrt(dx * dx + dy * dy);

	// AoE logic
	if (distance < targetPlayer->AOE_RADIUS * 2.0f)
    {
		// Take damage
        this->health -= targetPlayer->AOE_DAMAGE * deltaTime;

        // Visual feedback (Red)
        spriteRenderer.colour.r = 1.0f;
        spriteRenderer.colour.g = 0.0f;
    }
	else
	{
		// Reset color (Yellow)
		spriteRenderer.colour.g = 1.0f;
	}

	// Death Check
	if (this->health <= 0.0f) {
		this->isActive = false;
	}

	// Basically moves towards your player the further it is from the player the faster it moves
	AEVec2 direction{};
	AEVec2 displacement = targetPlayer->transform.position - this->transform.position;
	AEVec2Normalize(&direction,&displacement);
	//AEVec2 damped_displacement = ScaleVector(direction, 0.02f); // Dampen the movement
	this->transform.position = transform.position + direction * deltaTime * speed;
	std::cout << "Enemy - Update: Enemy Position (" << this->transform.position.x << ',' << this->transform.position.y << ")\n";
}