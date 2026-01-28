#include "pch.h"
#include "Enemy.h"
#include "Util.h"

void Enemy::Init(float startX, float startY)
{
    pos = { startX, startY };
    scale = 1.0f; // Size of enemy
    color = 0xFFFFFF00; // Yellow
    pMesh = Meshes::pTriangleMesh;
}

void Enemy::Update(float dt)
{
    // Enemy logic goes here (AI, movement, etc)
}

void Enemy::Draw()
{
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

    float r = ((color >> 16) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;
    float b = ((color) & 0xFF) / 255.0f;
    float a = ((color >> 24) & 0xFF) / 255.0f;
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxSetColorToMultiply(r, g, b, a);

    AEMtx33 transform, s, t;
    AEMtx33Scale(&s, scale, scale);
    AEMtx33Trans(&t, pos.x, pos.y);
    AEMtx33Concat(&transform, &t, &s);

    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(pMesh, AE_GFX_MDM_TRIANGLES);
}