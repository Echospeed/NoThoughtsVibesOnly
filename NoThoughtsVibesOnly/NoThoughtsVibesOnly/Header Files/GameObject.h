#pragma once
//Defines a class structure to manage game entities using the engine's math and graphics classes.
#include "AEEngine.h"
#include "SpriteRenderer.h"
#include "Transform.h"
#include "GameObjectType.h"

class GameObject {
    public:
    GameObject(AEVec2 pos = { 0.0f, 0.0f }, AEVec2 scale = { 50.0f, 50.0f }, f32 rot = 0.0f, AEGfxTexture* texture = nullptr, f32 width = 100.0f, f32 height = 100.0f); // Default Constructor
    ~GameObject(); //Destructor
    Transform transform;
	SpriteRenderer spriteRenderer;
    virtual void Start();
    virtual void Update(f32 deltaTime);
	void Draw();
    ObjectType ObjectType;

    private:
	void Load();
	void Init(AEVec2 pos, AEVec2 scale, f32 rot, AEGfxTexture* texture, f32 width, f32 height);
	void Free();
};