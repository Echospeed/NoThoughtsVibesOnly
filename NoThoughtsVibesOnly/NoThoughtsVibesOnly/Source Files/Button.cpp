#include "pch.hpp"
#include "Button.hpp"
#include "Transform.hpp"

Button::Button(s8 font, AEVec2 pos, AEVec2 size, ButtonFunction function, Colour colour, std::string text)
    : GameObject(pos, size, 0.0f, nullptr, size.x, size.y, STATE_MENU)
{
    this->textRenderer.font = font;
	this->textRenderer.position = pos;
	this->textRenderer.scale = size;
	this->textRenderer.SetText(text);

    this->spriteRenderer.width = this->transform.scale.x;
    this->spriteRenderer.height = this->transform.scale.y;
	this->spriteRenderer.colour = colour;
	this->ogColour = colour;
	this->tintColour.r = colour.r * 0.8f;
	this->tintColour.g = colour.g * 0.8f;
	this->tintColour.b = colour.b * 0.8f;

    this->collider.position = this->transform.position;
    this->collider.scale = this->transform.scale;
    this->onClick = function;
}

void Button::Update(f32 deltaTime)
{
    GameObject::Update(deltaTime);

    static_cast<void>(deltaTime);

	Mouse mouse;

	GetMouseWorldPosition(mouse.position.x, mouse.position.y);

    if (isOverlapping(this->collider, mouse))
    {
        // Tint the button when hovered
        this->spriteRenderer.colour = this->tintColour;

        if (AEInputCheckTriggered(AEVK_LBUTTON))
        {
            if (this->onClick)
				this->onClick();
        }
    }
    else
    {
        // Reset tint when not hovered
        this->spriteRenderer.colour = this->ogColour;
    }
    this->textRenderer.Draw();
}

Button::~Button()
{
    FreeTextRenderer(this->textRenderer);
}
