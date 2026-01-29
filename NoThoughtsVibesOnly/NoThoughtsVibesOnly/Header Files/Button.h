#pragma once
#include "AEEngine.h"
#include "Transform.h"
#include "Collider.h"
#include "SpriteRenderer.h"
#include "TextRenderer.h"

typedef void(*ButtonFunction)(void);

struct Button
{
    Transform transform{};
    SquareCollider collider{};
    SpriteRenderer sprite{};
    TextRenderer textRenderer{};
    const char* texturePath{};
	f32 r{}, g{}, b{}; // Tinted when hovered
    bool isHovered{ false };
    ButtonFunction onClick{ nullptr };
};

void LoadButton(Button& button, s8& font);

void InitButton(Button& button, const char* text, const char* texture, AEVec2 position, AEVec2 scale, ButtonFunction onClick, f32 r, f32 g, f32 b, TextAlignment textAlignment);

void InitButton(Button& button, const char* text, const char* texture, AEVec2 position, AEVec2 scale, ButtonFunction onClick, f32 r, f32 g, f32 b);

void UpdateButton(Button& button);

void DrawButton(Button& button);

void FreeButton(Button& button);