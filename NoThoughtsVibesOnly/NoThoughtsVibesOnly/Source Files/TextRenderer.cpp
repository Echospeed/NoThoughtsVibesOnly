#include "pch.hpp"
#include "TextRenderer.hpp"
#include "StateManager.hpp"

void TextRenderer::Draw()
{
	f32 width, height;
	AEGfxGetPrintSize(this->font, this->text.c_str(), 1.0f, &width, &height);

	f32 normX = position.x / (SCREEN_W / 2.0F);
	f32 normY = position.y / (SCREEN_H / 2.0F);

	f32 drawX{}, drawY{};
	if (this->alignment == ALIGN_CENTER)
	{
		// Center the text by adjusting the x position (default)
		drawX = normX - (width / 2.0f);
		drawY = normY - (height / 2.0f);
	}
	else if (this->alignment == ALIGN_RIGHT)
	{
		// Right align the text by adjusting the x position
		drawX = normX - (width / 2.0f);
		drawY = (height / 2.0f);
	}

	AEGfxPrint(this->font, this->text.c_str(), drawX, drawY, 1.0f, this->colour.r, this->colour.g, this->colour.b, 1.0f);

	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
	AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);
}

void LoadTextRenderer(TextRenderer& textRenderer, s8 font)
{
	textRenderer.font = font;
}

void InitTextRenderer(TextRenderer& textRenderer, const char* text, AEVec2 scale = { 1.0f, 1.0f }, f32 r = 1.0f, f32 g = 1.0f, f32 b = 1.0f)
{
	textRenderer.SetText(text);
	textRenderer.colour.r = r;
	textRenderer.colour.g = g;
	textRenderer.colour.b = b;
	textRenderer.scale = scale;
}

void DrawTextRenderer(const TextRenderer& textRenderer, AEVec2 position, f32 scale)
{
	f32 width, height;
	AEGfxGetPrintSize(textRenderer.font, textRenderer.text.c_str(), scale, &width, &height);

	f32 normX = position.x / (SCREEN_W / 2.0F);
	f32 normY = position.y / (SCREEN_H / 2.0F);

	f32 drawX{}, drawY{};
	if (textRenderer.alignment == ALIGN_CENTER)
	{
		// Center the text by adjusting the x position (default)
		drawX = normX - (width / 2.0f);
		drawY = normY - (height/ 2.0f);
	}
	else if (textRenderer.alignment == ALIGN_RIGHT)
	{
		// Right align the text by adjusting the x position
		drawX = normX - (width / 2.0f);
		drawY = (height / 2.0f);
	}

	AEGfxPrint(textRenderer.font, textRenderer.text.c_str(), drawX, drawY, scale, textRenderer.colour.r, textRenderer.colour.g, textRenderer.colour.b, 1.0f);
}

void FreeTextRenderer(TextRenderer& textRenderer)
{
	textRenderer.text.clear();
}