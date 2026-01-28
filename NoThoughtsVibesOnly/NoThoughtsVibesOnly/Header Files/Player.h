#pragma once
#include "pch.h"

struct Player
{
    AEVec2 pos;
    AEVec2 velocity;
    float scale;
    float speed;
    unsigned int color;
    AEGfxVertexList* pMesh;

    // Initialize variables
    void Init();

    // Update logic
    void Update(float dt, float worldWidth, float worldHeight);

    // Render the player
    void Draw();
};