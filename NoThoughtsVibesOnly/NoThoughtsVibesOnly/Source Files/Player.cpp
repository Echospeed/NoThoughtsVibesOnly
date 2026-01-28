#include "pch.h"
#include "Player.h"
#include "Util.h"

void Player::Init()
{
    pos = { 0.0f, 0.0f };
    velocity = { 0.0f, 0.0f };
    scale = 50.0f; // Player size
    speed = 300.0f; // Player speed
    color = 0xFFFF0000; // Red
    pMesh = Meshes::pSquareCOriMesh;
}

void Player::Update(float dt, float worldWidth, float worldHeight)
{
    // Reset Velocity
    velocity = { 0.0f, 0.0f };

    // Player Input (WASD)
    if (AEInputCheckCurr(AEVK_W)) velocity.y += speed;
    if (AEInputCheckCurr(AEVK_S)) velocity.y -= speed;
    if (AEInputCheckCurr(AEVK_A)) velocity.x -= speed;
    if (AEInputCheckCurr(AEVK_D)) velocity.x += speed;

    // Apply Movement
    pos.x += velocity.x * dt;
    pos.y += velocity.y * dt;

    // Boundary Logic
    float halfWorldWidth = worldWidth / 2.0f;
    float halfWorldHeight = worldHeight / 2.0f;
    float halfPlayerSize = scale / 2.0f;

    if (pos.x > halfWorldWidth - halfPlayerSize) pos.x = halfWorldWidth - halfPlayerSize;
    if (pos.x < -halfWorldWidth + halfPlayerSize) pos.x = -halfWorldWidth + halfPlayerSize;
    if (pos.y > halfWorldHeight - halfPlayerSize) pos.y = halfWorldHeight - halfPlayerSize;
    if (pos.y < -halfWorldHeight + halfPlayerSize) pos.y = -halfWorldHeight + halfPlayerSize;
}

void Player::Draw()
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