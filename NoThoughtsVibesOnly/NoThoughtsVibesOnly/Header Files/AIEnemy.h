#pragma once
#include "AEEngine.h"
#include "GameObject.h"
#include "Player.h" // reference to player

class AIEnemy : public GameObject
{
public:
    // Constructor takes pointer to player
    AIEnemy(Player* target);

    void Start();
    void Update(f32 deltaTime);

private:
    Player* targetPlayer{ nullptr }; // reference to the player
    float speed{ 300.0f };           // movement speed
    float stopDistance{ 200.0f };     // minimum distance from player
    float velX{ 0.0f };
    float velY{ 0.0f };
};
