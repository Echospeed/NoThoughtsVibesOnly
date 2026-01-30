#pragma once
#include "AEEngine.h"
#include "GameObject.h"
#include "Player.h"

class Enemy : public GameObject
{
public:
	// Constructor takes pointer to player
	Enemy(Player* target);

	void Start();
	void Update(f32 deltaTime);

	// Stats
	float health{ 200.0f };

private:
	Player* targetPlayer{ nullptr }; // Reference to the player
	float speed{ 100.0f };           // Movement speed
	float stopDistance{ 200.0f };    // Minimum distance from player
	float velX{ 0.0f };
	float velY{ 0.0f };
};