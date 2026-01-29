#include "AIEnemy.h"
#include <cmath>
#include <iostream>
#include "GamePage.h"

const float WORLD_WIDTH = 2000.0f;
const float WORLD_HEIGHT = 2000.0f;

AIEnemy::AIEnemy(Player* target)
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

void AIEnemy::Start()
{
    // Random starting position inside red box
    float halfWorldWidth = WORLD_WIDTH / 2.0f;
    float halfWorldHeight = WORLD_HEIGHT / 2.0f;

    transform.position = {
        (float)(rand() % (int)WORLD_WIDTH - halfWorldWidth),
        (float)(rand() % (int)WORLD_HEIGHT - halfWorldHeight)
    };

    transform.scale = { 50.0f, 50.0f };
    transform.rotation = 0.0f;

    spriteRenderer.colour = { 0.0f, 1.0f, 0.0f }; // green
    spriteRenderer.meshType = MESH_CIRCLE;

    std::cout << "AIEnemy spawned at ("
        << transform.position.x << ", "
        << transform.position.y << ")\n";
}

void AIEnemy::Update(f32 deltaTime)
{
    if (!targetPlayer) return; // safety check

    float dx = targetPlayer->transform.position.x - transform.position.x;
    float dy = targetPlayer->transform.position.y - transform.position.y;
    float distance = std::sqrt(dx * dx + dy * dy);

    // --- Flee logic: move away if player is too close ---
    if (distance < 200.0f) // flee if within 200 units
    {
        float nx = -dx / distance;
        float ny = -dy / distance;

        velX = nx * speed;
        velY = ny * speed;
    }

    // --- Update position with velocity ---
    transform.position.x += velX * deltaTime;
    transform.position.y += velY * deltaTime;

    // --- Rotate to face movement direction ---
    if (velX != 0.0f || velY != 0.0f)
        transform.rotation = atan2f(velY, velX);

    // --- Wall bounce ---
    float halfWorldWidth = WORLD_WIDTH / 2.0f;
    float halfWorldHeight = WORLD_HEIGHT / 2.0f;
    float halfEnemyWidth = transform.scale.x / 2.0f;
    float halfEnemyHeight = transform.scale.y / 2.0f;

    if (transform.position.x > halfWorldWidth - halfEnemyWidth)
    {
        transform.position.x = halfWorldWidth - halfEnemyWidth;
        velX = -velX; // bounce horizontally
    }
    if (transform.position.x < -halfWorldWidth + halfEnemyWidth)
    {
        transform.position.x = -halfWorldWidth + halfEnemyWidth;
        velX = -velX;
    }
    if (transform.position.y > halfWorldHeight - halfEnemyHeight)
    {
        transform.position.y = halfWorldHeight - halfEnemyHeight;
        velY = -velY; // bounce vertically
    }
    if (transform.position.y < -halfWorldHeight + halfEnemyHeight)
    {
        transform.position.y = -halfWorldHeight + halfEnemyHeight;
        velY = -velY;
    }
}
