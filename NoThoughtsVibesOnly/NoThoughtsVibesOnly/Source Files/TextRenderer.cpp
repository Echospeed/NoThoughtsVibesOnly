#include "TextRenderer.h"
#include "StateManager.h"

void LoadTextRenderer(TextRenderer& textRenderer, s8 font)
{
	textRenderer.font = font;
}

void InitTextRenderer(TextRenderer& textRenderer, const char* text, f32 scale = 1.0f, f32 r = 1.0f, f32 g = 1.0f, f32 b = 1.0f)
{
	textRenderer.text = text;
	textRenderer.r = r;
	textRenderer.g = g;
	textRenderer.b = b;
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

	AEGfxPrint(textRenderer.font, textRenderer.text.c_str(), drawX, drawY, scale, textRenderer.r, textRenderer.g, textRenderer.b, 1.0f);
}

void DrawTextRenderer(const TextRenderer& textRenderer, AEVec2 position, f32 scale, TextAlignment alignment = ALIGN_CENTER)
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
		drawY = normY - (height / 2.0f);
	}
	else if (textRenderer.alignment == ALIGN_RIGHT)
	{
		// Right align the text by adjusting the x position
		drawX = normX - (width / 2.0f);
		drawY = (height / 2.0f);
	}

	AEGfxPrint(textRenderer.font, textRenderer.text.c_str(), drawX, drawY, scale, textRenderer.r, textRenderer.g, textRenderer.b, 1.0f);
}

void FreeTextRenderer(TextRenderer& textRenderer)
{
	AEGfxDestroyFont(textRenderer.font);
}