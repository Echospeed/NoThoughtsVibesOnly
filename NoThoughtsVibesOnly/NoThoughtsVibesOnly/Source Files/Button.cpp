// ============================================================================
// Button.cpp - Clickable UI Button Implementation
// ============================================================================
// See Button.hpp for full documentation and usage examples.
// ============================================================================

#include "pch.hpp"
#include "Button.hpp"
#include "Transform.hpp"

// ============================================================================
// Constructor
// ============================================================================
// Initialises the button's transform, sprite, collider, text, and colours.
// The tint colour is auto-computed as 80% of the base colour for hover effect.
// ============================================================================
Button::Button(s8 font, AEVec2 pos, AEVec2 size,
    ButtonFunction function, Colour colour, std::string text)
    : GameObject(pos, size, 0.0f, nullptr, size.x, size.y, STATE_MENU)
{
    // --- Text setup ---
    textRenderer = TextRenderer(font, 1.0f, pos, { 1.0f, 1.0f, 1.0f, 1.0f });
	textRenderer << text;

    // --- Sprite setup ---
    spriteRenderer.width = transform.scale.x;
    spriteRenderer.height = transform.scale.y;
    spriteRenderer.colour = colour;

    // --- Colour state ---
    ogColour = colour;
    tintColour = {
        colour.r * 0.8f,
        colour.g * 0.8f,
        colour.b * 0.8f,
        colour.a           // Keep full alpha on hover
    };

    // --- Collider (AABB for mouse-over detection) ---
    collider.position = transform.position;
    collider.scale = transform.scale;

    // --- Callback ---
    onClick = function;
}

// ============================================================================
// Update
// ============================================================================
// Each frame:
//   1. Skips logic entirely if the button is inactive.
//   2. Gets current mouse world position.
//   3. Applies hover tint if the mouse overlaps the collider.
//   4. Fires onClick callback on left-click while hovered.
//   5. Draws the text label.
// ============================================================================
void Button::Update(f32 deltaTime)
{
    if (!isActive) return;

    // Let base class run any common update logic (draws sprite)
    GameObject::Update(deltaTime);

    // Suppress unused parameter warning (deltaTime not used directly here)
    static_cast<void>(deltaTime);

    // Get current mouse position in world space
    Mouse mouse;
    GetMouseWorldPosition(mouse.position.x, mouse.position.y);

    if (isOverlapping(collider, mouse))
    {
        // Apply hover tint
        spriteRenderer.colour = tintColour;

        // Trigger callback on left-click
        if (AEInputCheckTriggered(AEVK_LBUTTON) && onClick)
        {
            onClick();
        }
    }
    else
    {
        // Restore normal colour when not hovered
        spriteRenderer.colour = ogColour;
    }

    // Draw text label on top of the button background
    textRenderer.Draw();
}

// ============================================================================
// Destructor
// ============================================================================
Button::~Button()
{
    
}