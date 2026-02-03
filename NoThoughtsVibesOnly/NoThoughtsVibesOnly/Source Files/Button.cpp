#include "pch.h"
#include "Button.h"
#include "Transform.h"

void LoadButton(Button& button, s8 &font)
{
    if (font == 0)
    {
	    printf("Button font load fail.\n");
    }
    LoadTextRenderer(button.textRenderer, font);
}

void InitButton(Button& button, const char* text, const char* texture, AEVec2 position, AEVec2 scale, ButtonFunction onClick, f32 r, f32 g, f32 b, TextAlignment textAlignment = ALIGN_CENTER)
{
    button.transform.position = position;
    button.transform.scale = scale;
    button.onClick = onClick;
    
    // Default Colors
    button.sprite.colour.r = r; 
    button.sprite.colour.g = g;
    button.sprite.colour.b = b;

    // Collider 
	button.collider.position = position;
	button.collider.scale = scale;

	// Text Renderer
    button.textRenderer.text = text;
	button.textRenderer.alignment = textAlignment;
	InitTextRenderer(button.textRenderer, button.textRenderer.text.c_str(), 1.0f, 1.0f, 1.0f, 1.0f);

	// Sprite Renderer
    button.texturePath = texture;
	InitSpriteRenderer(button.sprite, button.texturePath, scale.x, scale.y, MESH_SQUARE);
}

void InitButton(Button& button, const char* text, const char* texture, AEVec2 position, AEVec2 scale, ButtonFunction onClick, f32 r, f32 g, f32 b)
{
	InitButton(button, text, texture, position, scale, onClick, r, g, b, ALIGN_CENTER);
}

void UpdateButton(Button& button)
{
    // Potentially add update logic here in the future
	TransformMovement(button.transform);
}

void DrawButton(Button& button)
{
	DrawSpriteRenderer(button.sprite, button.transform);
	DrawTextRenderer(button.textRenderer, button.transform.position, button.textRenderer.scale);
}

void FreeButton(Button& button)
{
    FreeSpriteRenderer(button.sprite);
	FreeTextRenderer(button.textRenderer);
}