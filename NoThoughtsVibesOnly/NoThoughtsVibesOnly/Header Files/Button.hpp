#pragma once
// ============================================================================
// Button.hpp - Clickable UI Button
// ============================================================================
// A Button is a GameObject with a label, click callback, hover tint, and
// an AABB collider for mouse-over detection.
//
// USAGE EXAMPLE:
// ----------------------------------------------------------------------------
//   // Define a callback function (must match ButtonFunction signature):
//   void OnRestartClicked() { StateManagerChangeState(STATE_RESTART); }
//   void OnQuitClicked()    { StateManagerChangeState(STATE_MENU); }
//
//   // Create a button (in Init):
//   Button* restartBtn = new Button(
//       fontPath,                    // s8 font handle from AEGfxCreateFont()
//       { 0.0f, -100.0f },          // AEVec2 world position
//       { 300.0f, 75.0f },          // AEVec2 size (width, height)
//       OnRestartClicked,            // Callback invoked on left-click
//       { 0.0f, 0.6f, 0.0f, 1.0f}, // Colour (r,g,b,a)
//       "RESTART"                    // Label text
//   );
//
//   // In Update (buttons self-update including hover/click detection):
//   restartBtn->Update(deltaTime);
//
//   // In Free:
//   delete restartBtn;
//   restartBtn = nullptr;
//
// ACTIVATION:
// ----------------------------------------------------------------------------
//   restartBtn->isActive = true;  // Enable button (visible + clickable)
//   restartBtn->isActive = false; // Hide button (skipped in Update)
// ============================================================================

#include "AEEngine.h"
#include "GameObject.hpp"
#include "Transform.hpp"
#include "Collider.hpp"
#include "SpriteRenderer.hpp"
#include "TextRenderer.hpp"

// Function pointer type for button click callbacks (void, no parameters)
typedef void(*ButtonFunction)(void);

// ============================================================================
// class Button : public GameObject
// ============================================================================
// Renders a coloured rectangle with a text label.
// Automatically detects mouse hover and triggers onClick on left-click.
// Applies a darker tint colour when the mouse is over the button.
// ============================================================================
class Button : public GameObject
{
public:
    // ------------------------------------------------------------------------
    // Colour state
    // ------------------------------------------------------------------------
    Colour ogColour{};       // Base colour used when not hovered
    Colour tintColour{};     // Darkened colour applied on mouse-over (auto-computed)

    // ------------------------------------------------------------------------
    // onClick - Function called when the button is left-clicked while hovered.
    // Assign a matching ButtonFunction (void function, no parameters).
    // ------------------------------------------------------------------------
    ButtonFunction onClick{ nullptr };

    // ------------------------------------------------------------------------
    // textRenderer - Draws the button label text.
    // Initialised from the label string passed to the constructor.
    // ------------------------------------------------------------------------
    TextRenderer textRenderer{};

    // ------------------------------------------------------------------------
    // collider - AABB used for mouse-over detection (isOverlapping).
    // Automatically sized and positioned to match the button's transform.
    // ------------------------------------------------------------------------
    SquareCollider collider{};

    // Default constructor (required for stack-allocated buttons)
    Button() = default;

    // ------------------------------------------------------------------------
    // Button Constructor
    // font     : Font handle from AEGfxCreateFont()
    // pos      : World-space position of the button center
    // size     : Width and height of the button rectangle
    // function : Callback triggered on left-click
    // colour   : Fill colour of the button background
    // text     : Label string drawn centered on the button
    // ------------------------------------------------------------------------
    Button(s8 font, AEVec2 pos, AEVec2 size, ButtonFunction function,
        Colour colour, std::string text = "");

    // ------------------------------------------------------------------------
    // Update - Handles hover detection and click input.
    // Automatically draws the text label each frame.
    // Only runs if isActive == true.
    // ------------------------------------------------------------------------
    void Update(f32 deltaTime) override;

    // Destructor - frees the text renderer
    ~Button() override;
};