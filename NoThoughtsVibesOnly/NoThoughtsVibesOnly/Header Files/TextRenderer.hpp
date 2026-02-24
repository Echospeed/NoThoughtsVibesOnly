#pragma once
#include "AEEngine.h"
#include "GameObject.hpp"
#include <string>
#include <sstream>

enum TextAlignment
{
	ALIGN_LEFT,
	ALIGN_CENTER,
	ALIGN_RIGHT
};

class TextRenderer
{

public:
	s8 font{};
	AEVec2 scale{};
	Colour colour{};
	AEVec2 position{};
	std::string text{}; // Shift to private once done
	TextAlignment alignment{ ALIGN_CENTER };

	template <typename... Args>
	void SetText(const Args&... args)
	{
		std::ostringstream stream;

		// This "folds" all arguments (int, float, string) into the stream
		(stream << ... << args);

		// Update the struct's text variable directly
		text = stream.str();
	}

	void Draw();
};

void LoadTextRenderer(TextRenderer& textRenderer, s8 font);

void InitTextRenderer(TextRenderer& textRenderer, const char* text, AEVec2 scale, f32 r, f32 g, f32 b);

void DrawTextRenderer(const TextRenderer& textRenderer, AEVec2 position, f32 scale);

void FreeTextRenderer(TextRenderer& textRenderer);
