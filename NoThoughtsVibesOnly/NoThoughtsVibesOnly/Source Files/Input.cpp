// ============================================================================
// Input.cpp - Mouse Input Helpers
// ============================================================================
// Converts Alpha Engine screen-space mouse coordinates into world-space and
// screen-space positions for use in gameplay and UI hit-testing.
//
// WORLD POSITION:
//   Takes screen pixel (0,0 = top-left) and converts to world space by
//   centering on screen and offsetting by current camera position.
//   Used for: bullet aiming, button hover detection in world space.
//
// SCREEN POSITION:
//   Returns raw pixel coordinates. Useful for raw screen-space UI.
// ============================================================================

#include "pch.hpp"
#include "Input.hpp"
#include "StateManager.hpp"

// ============================================================================
// GetMouseWorldPosition
// ============================================================================
// Converts the current mouse pixel position to world-space coordinates.
// Accounts for the current AE camera offset (which moves with the player).
//
// outWorldX / outWorldY : Filled with the world-space position.
// ============================================================================
void GetMouseWorldPosition(f32& outWorldX, f32& outWorldY)
{
    s32 mouseX, mouseY;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    f32 camX, camY;
    AEGfxGetCamPosition(&camX, &camY);

    // Convert pixel -> NDC -> world:
    //   (mouseX - halfScreenW) = screen-center-relative X
    //   + camX = world offset
    outWorldX = (static_cast<f32>(mouseX) - SCREEN_W / 2.0f) + camX;
    outWorldY = -(static_cast<f32>(mouseY) - SCREEN_H / 2.0f) + camY; // Y is flipped
}

// ============================================================================
// GetMouseScreenPosition
// ============================================================================
// Returns raw screen pixel coordinates (top-left origin).
// ============================================================================
void GetMouseScreenPosition(f32& outScreenX, f32& outScreenY)
{
    s32 mouseX, mouseY;
    AEInputGetCursorPosition(&mouseX, &mouseY);
    outScreenX = static_cast<f32>(mouseX);
    outScreenY = static_cast<f32>(mouseY);
}