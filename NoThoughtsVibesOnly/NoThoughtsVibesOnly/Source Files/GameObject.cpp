// ============================================================================
// GameObject.cpp - FIXED VERSION (No Warnings)
// ============================================================================
// COPY AND PASTE THIS ENTIRE FILE
// 
// FIX: Added (void)deltaTime to suppress C4100 unreferenced parameter warning
// ============================================================================

#include "pch.hpp"
#include "GameObject.hpp"
#include "GamePage.hpp"

GameObject::GameObject(AEVec2 pos, AEVec2 scale, f32 rot, AEGfxTexture* texture, f32 width, f32 height, GameState state)
{
	Init(pos, scale, rot, texture, width, height);
	switch (state)
	{
	case STATE_MENU:
		mainPageObj.push_back(this);
		break;
	case STATE_PLAYING:
		gamePageObj.push_back(this);
		break;
	default:
		break;
	}
}

GameObject::~GameObject()
{
	Free();
}

void GameObject::Init(AEVec2 pos, AEVec2 scale, f32 rot, AEGfxTexture* texture, f32 width, f32 height)
{
	// Programmer Defined Values
	transform.position = { pos.x, pos.y };
	transform.scale = { scale.x, scale.y };
	transform.rotation = rot;

	// Programmer Defined Sprite Renderer Values
	spriteRenderer.texture = texture;
	spriteRenderer.width = width;
	spriteRenderer.height = height;
}

void GameObject::Start()
{
	// Override in derived classes
}

// ============================================================================
// FIX: Suppress C4100 unreferenced parameter warning
// ============================================================================
void GameObject::Update(f32 deltaTime)
{
	(void)deltaTime; // Suppress unused parameter warning
	Draw();
}

void GameObject::Draw()
{
	DrawSpriteRenderer(spriteRenderer, transform);
}

void GameObject::Free()
{
	FreeSpriteRenderer(spriteRenderer);
}
