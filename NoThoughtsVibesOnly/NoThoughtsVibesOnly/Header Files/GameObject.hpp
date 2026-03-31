#pragma once
// Author: Chia Wenjie
// Co-Author: John Chiow

#include "AEEngine.h"
#include "SpriteRenderer.hpp"
#include "Transform.hpp"
#include "GameObjectType.hpp"
#include "StateManager.hpp"

class GameObject
{
public:
    // STATE_PLAYING is the sensible default - most GameObjects are game entities.
    // Pass STATE_MENU explicitly for menu objects.
    GameObject(AEVec2 pos = { 0.0f, 0.0f }, AEVec2 scale = { 50.0f, 50.0f }, f32 rot = 0.0f,
        AEGfxTexture* texture = nullptr, f32 width = 100.0f, f32 height = 100.0f,
        GameState state = STATE_PLAYING);
    virtual ~GameObject();

    Transform      transform;
    SpriteRenderer spriteRenderer;

    virtual void Start();
    virtual void Update(f32 deltaTime);
    void Draw();

    ObjectType ObjectType;
    bool isActive = true;

private:
    void Init(AEVec2 pos, AEVec2 scale, f32 rot, AEGfxTexture* texture, f32 width, f32 height);
    void Free();
};