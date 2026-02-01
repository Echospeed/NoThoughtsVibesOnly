#pragma once
#include "AEEngine.h"
#include "Transform.h"
#include "Colour.h"

enum MeshType
{
	MESH_SQUARE,
	MESH_TRIANGLE,
	MESH_CIRCLE
};

struct SpriteRenderer {

	AEGfxTexture* texture{ nullptr };
	f32 width{};
	f32 height{};
	Colour colour{};
	MeshType meshType{ MESH_SQUARE };
};

void InitSpriteRenderer(SpriteRenderer& spriteRenderer, const char* texturePath, f32 width, f32 height, MeshType mesh);

void DrawSpriteRenderer(const SpriteRenderer& spriteRenderer, Transform& transform);

void FreeSpriteRenderer(SpriteRenderer& spriteRenderer);