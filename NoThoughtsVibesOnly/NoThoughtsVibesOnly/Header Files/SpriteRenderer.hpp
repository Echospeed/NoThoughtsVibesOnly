#pragma once
#include "AEEngine.h"
#include "Transform.hpp"
#include "Colour.hpp"

enum MeshType
{
	MESH_SQUARE,
	MESH_TRIANGLE,
	MESH_CIRCLE
};

struct SpriteRenderer 
{

	AEGfxTexture* texture{ nullptr };
	f32 width{};
	f32 height{};
	Colour colour{0.f, 0.f, 0.f, 1.0f};
	MeshType meshType{ MESH_SQUARE };
};

void InitSpriteRenderer(SpriteRenderer& spriteRenderer, const char* texturePath, f32 width, f32 height, MeshType mesh);

void DrawSpriteRenderer(const SpriteRenderer& spriteRenderer, Transform& transform);

void FreeSpriteRenderer(SpriteRenderer& spriteRenderer);