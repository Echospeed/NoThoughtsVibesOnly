// Implements the rendering logic using the Global Functions (AEGfxSetTransform, AEGfxMeshDraw) listed in the documentation
#include "pch.h"
#include "GameObject.h"

GameObject::GameObject() :
    pos{ 0.0f, 0.0f }, velocity{ 0.0f, 0.0f },
    rotation(0.0f), scale(1.0f),
    color(0xFFFFFFFF), pMesh(nullptr) {
}

void GameObject::Update(float dt) {
    // Update position based on frame time (dt)
    pos.x += velocity.x * dt;
    pos.y += velocity.y * dt;
}

void GameObject::Draw() {
    // Set engine to Color Render Mode
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);

    // Matrix Transformation: Scale -> Rotate -> Translate
    AEMtx33 transform, s, r, t;
    AEMtx33Scale(&s, scale, scale);
    AEMtx33Rot(&r, rotation);
    AEMtx33Trans(&t, pos.x, pos.y);

    // Combine matrices for the final GPU transform
    AEMtx33Concat(&transform, &r, &s);
    AEMtx33Concat(&transform, &t, &transform);
    AEGfxSetTransform(transform.m);

    // Set Tint for the mesh using bitwise extraction
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    AEGfxSetColorToMultiply(
        ((color >> 16) & 0xFF) / 255.0f, // Red
        ((color >> 8) & 0xFF) / 255.0f,  // Green
        ((color) & 0xFF) / 255.0f,       // Blue
        ((color >> 24) & 0xFF) / 255.0f  // Alpha
    );

    // Render the mesh if it exists
    if (pMesh) {
        AEGfxMeshDraw(pMesh, AE_GFX_MDM_TRIANGLES);
    }
}