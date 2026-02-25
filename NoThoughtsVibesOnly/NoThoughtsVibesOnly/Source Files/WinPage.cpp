// ============================================================================
// WinPage.cpp - Victory Screen
// ============================================================================
// Shown when the player clears all waves.
// Displays a "YOU WIN!" text that grows over time.
//
// BUTTON HANDLING:
// ----------------------------------------------------------------------------
//   The Win button (currently commented out) can be re-enabled to:
//     - Return to Main Menu:   callback = StateManagerMenuPage
//     - Restart the game:      callback = OnWinRestartClicked
//
//   To re-enable:
//     1. Uncomment WinButton lines in Load, Init, Update, Draw, Free.
//     2. Set desired callback in WinPage_Init().
//
// USAGE:
// ----------------------------------------------------------------------------
//   Entered automatically via StateManagerChangeState(STATE_WIN).
//   Triggered in Game_Update() when all enemies are cleared.
// ============================================================================

#include "pch.hpp"
#include "WinPage.hpp"
#include "AEEngine.h"
#include "Util.hpp"
#include "Button.hpp"

// ============================================================================
// File-scope state
// ============================================================================
namespace
{
    Mouse worldMouse;                  // Mouse position for button hover
    s8    fontPath{};                  // Font handle from AEGfxCreateFont()

    // Text animation parameters
    f32 InitialScale = 1.5f;          // Scale at the start of the animation
    f32 FinalScale = 3.5f;          // Scale at the end of the animation
    f32 dt = 0.0f;          // Delta time per frame
    f32 timer = 0.0f;          // Elapsed time since page was entered
}

// UI elements
TextRenderer WinText;                  // Renders the "YOU WIN!" message
// Button WinButton;                   // Uncomment to re-enable button

// ============================================================================
// Button Callbacks
// ============================================================================

// Return to menu after winning
void OnWinMenuClicked()
{
    StateManagerChangeState(STATE_MENU);
}

// Restart the game from the win screen
void OnWinRestartClicked()
{
    StateManagerChangeState(STATE_RESTART);
}

// ============================================================================
// WinPage_Load
// ============================================================================
// Loads fonts and creates mesh resources. Called once on state entry.
// ============================================================================
void WinPage_Load()
{
    fontPath = AEGfxCreateFont("Assets/buggy-font.ttf", 30);
    LoadTextRenderer(WinText, fontPath);
    // LoadButton(WinButton, fontPath); // Uncomment to enable button

    Meshes::CreateSquareCenterOriginMesh();
}

// ============================================================================
// WinPage_Init
// ============================================================================
// Resets animation and initialises UI. Called every time this state is entered.
// ============================================================================
void WinPage_Init()
{
    AEGfxSetBackgroundColor(0.1f, 0.1f, 0.15f);

    // Reset animation
    InitialScale = 1.5f;
    timer = 0.0f;

    // "YOU WIN!" in red (change colour values to customise)
    InitTextRenderer(WinText, "YOU WIN!", { 1.0f, 1.0f }, 1.0f, 0.0f, 0.0f);

    // To enable the win button, uncomment:
    // InitButton(WinButton, "MAIN MENU", nullptr,
    //     { 0.0f, -200.0f }, { 300.0f, 75.0f },
    //     OnWinMenuClicked, 0.0f, 0.6f, 0.0f);
}

// ============================================================================
// WinPage_Update
// ============================================================================
// Drives the text grow animation and button interaction.
// ============================================================================
void WinPage_Update()
{
    // Center camera on UI
    AEGfxSetCamPosition(0.0f, 0.0f);

    GetMouseWorldPosition(worldMouse.position.x, worldMouse.position.y);

    dt = (f32)AEFrameRateControllerGetFrameTime();
    timer += dt;

    // Smoothly scale text up toward FinalScale over 6 seconds
    if (timer < 6.0f)
    {
        InitialScale += (FinalScale - InitialScale) * 0.6f * dt;
    }

    // --- Win Button (uncomment to enable) ---
    // WinButton.isHovered = isOverlapping(WinButton.collider, worldMouse);
    // if (WinButton.isHovered && AEInputCheckTriggered(AEVK_LBUTTON))
    //     WinButton.onClick();
}

// ============================================================================
// WinPage_Draw
// ============================================================================
void WinPage_Draw()
{
    DrawTextRenderer(WinText, { 0.0f, 250.0f }, InitialScale);
    // DrawButton(WinButton); // Uncomment to render the button
}

// ============================================================================
// WinPage_Free
// ============================================================================
void WinPage_Free()
{
    FreeTextRenderer(WinText);
    // FreeButton(WinButton); // Uncomment if button is enabled
}

// ============================================================================
// WinPage_Unload
// ============================================================================
void WinPage_Unload()
{
    AEGfxDestroyFont(fontPath);
    Meshes::FreeMeshes();
}