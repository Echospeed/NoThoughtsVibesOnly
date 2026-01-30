#pragma once
#include "AEEngine.h"
#include "GameObject.h"
#include "Player.h"

class AIEnemy : public GameObject
{
public:
    // Constructor takes pointer to player
    AIEnemy(Player* target);

    void Start();
    void Update(f32 deltaTime);

    // Stats
    float health{ 100.0f };

private:
    Player* targetPlayer{ nullptr }; // Reference to the player
    float speed{ 200.0f };           // Movement speed
    float stopDistance{ 200.0f };    // Minimum distance from player
    float velX{ 0.0f };
    float velY{ 0.0f };
};