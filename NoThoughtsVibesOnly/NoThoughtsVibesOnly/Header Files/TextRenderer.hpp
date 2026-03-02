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

	template <typename T>
	TextRenderer& operator<<(const T& value)
	{
		std::ostringstream stream;	

		stream << value;

		// Update the struct's text variable directly
		this->text += stream.str();

		return *this;
	}

	void Draw();
};

void LoadTextRenderer(TextRenderer& textRenderer, s8 font);

void InitTextRenderer(TextRenderer& textRenderer, const char* text, AEVec2 scale, f32 r, f32 g, f32 b);

void DrawTextRenderer(const TextRenderer& textRenderer, AEVec2 position, f32 scale);

void FreeTextRenderer(TextRenderer& textRenderer);
