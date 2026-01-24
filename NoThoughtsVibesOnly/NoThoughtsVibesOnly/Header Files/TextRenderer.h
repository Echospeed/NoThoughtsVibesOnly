#pragma once
#include "AEEngine.h"
#include <string>

enum TextAlignment
{
	ALIGN_LEFT,
	ALIGN_CENTER,
	ALIGN_RIGHT
};

struct TextRenderer
{
	s8 font;
	std::string text;
	AEVec2 position{};
	f32 scale{};
	f32 r; 
	f32 g; 
	f32 b;
	TextAlignment alignment{ ALIGN_CENTER };
};

void LoadTextRenderer(TextRenderer& textRenderer, s8 font);

void InitTextRenderer(TextRenderer& textRenderer, const char* text, f32 scale, f32 r, f32 g, f32 b);

void DrawTextRenderer(const TextRenderer& textRenderer, AEVec2 position, f32 scale);

void DrawTextRenderer(const TextRenderer& textRenderer, AEVec2 position, f32 scale, TextAlignment alignment);

void FreeTextRenderer(TextRenderer& textRenderer);