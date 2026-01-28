#pragma once
#include "pch.h"

struct Enemy
{
    AEVec2 pos;
    float scale;
    unsigned int color;
    AEGfxVertexList* pMesh;

    // Initialize with specific position
    void Init(float startX, float startY);

    // Update logic
    void Update(float dt);

    // Render the enemy
    void Draw();
};