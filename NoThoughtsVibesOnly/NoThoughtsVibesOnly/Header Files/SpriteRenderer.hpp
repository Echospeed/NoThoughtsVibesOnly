#pragma once
// Author: Chia Wenjie

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
	/*-------------Single Sprite----------------*/
	AEGfxTexture* texture{ nullptr };
	f32 width{};
	f32 height{};
	Colour colour{0.f, 0.f, 0.f, 1.0f};
	MeshType meshType{ MESH_SQUARE };

	/*---------------Animation------------------*/
	AEGfxVertexList* customMesh{ nullptr }; // Overrides the default square
	f32 uOffset{ 0.0f }; // Horizontal slide percentage
	f32 vOffset{ 0.0f }; // Vertical slide percentage

	SpriteRenderer() = default;
};

void InitSpriteRenderer(SpriteRenderer& spriteRenderer, const char* texturePath, f32 width, f32 height, MeshType mesh);

void DrawSpriteRenderer(const SpriteRenderer& spriteRenderer, Transform& transform);

AEGfxVertexList* CreateSpriteSheetMesh(f32 frameWidth, f32 frameHeight, f32 texWidth, f32 texHeight);

void FreeSpriteRenderer(SpriteRenderer& spriteRenderer);