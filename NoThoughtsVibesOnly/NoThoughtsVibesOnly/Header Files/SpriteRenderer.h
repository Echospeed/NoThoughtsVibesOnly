#pragma once
#include "AEEngine.h"
#include "Transform.h"

struct SpriteRenderer {

	AEGfxTexture* texture;
	f32 width;
	f32 height;
};

void InitSpriteRenderer(SpriteRenderer& spriteRenderer, const char* texturePath, f32 width, f32 height);

void DrawSpriteRenderer(const SpriteRenderer& spriteRenderer, Transform& transform, f32 r, f32 g, f32 b);

void FreeSpriteRenderer(SpriteRenderer& spriteRenderer);