#pragma once
#include "AEEngine.h"
#include "GameObject.hpp"
#include "Transform.hpp"
#include "Collider.hpp"
#include "SpriteRenderer.hpp"
#include "TextRenderer.hpp"

typedef void(*ButtonFunction)(void);

class Button : public GameObject
{
public:
    // Public Class
    //bool isHovered{ false };
	Colour ogColour{};
    Colour tintColour{};
    ButtonFunction onClick{ nullptr };
    TextRenderer textRenderer{};
    SquareCollider collider{};

	// Constructor
    Button() = default;

    Button(s8 font, AEVec2 pos, AEVec2 size, ButtonFunction function, Colour colour, std::string text = "");

    void Update(f32 deltaTime) override;

	void Draw();

    ~Button();
};