// ============================================================================
// FinishPage.cpp - Game Over / Lose Screen
// ============================================================================
// Shown when the player's health reaches zero.
// Displays a "YOU LOSE!" text that grows over time.
//
// BUTTON HANDLING:
// ----------------------------------------------------------------------------
//   The Restart button (currently commented out) calls StateManagerMenuPage()
//   to return to the main menu. To re-enable the full button:
//     1. Uncomment RestartButton lines in Load, Init, Update, Draw, Free.
//     2. RestartButton calls OnFinishRestartClicked() -> STATE_RESTART
//     3. Or swap to StateManagerMenuPage() to go back to the menu instead.
//
// USAGE:
// ----------------------------------------------------------------------------
//   This page is automatically entered via:
//       StateManagerChangeState(STATE_FINISH);
//   Triggered in Game_Update() when player->health <= 0.
// ============================================================================

#include "pch.hpp"
#include "FinishPage.hpp"
#include "AEEngine.h"
#include "Util.hpp"
#include "Button.hpp"

// ============================================================================
// File-scope (anonymous namespace) state
// ============================================================================
namespace
{
    Mouse     worldMouse;              // Stores mouse position for button hover
    s8        fontPath{};             // Font handle loaded in FinishPage_Load()

    // Text animation parameters
    f32 InitialScale = 1.5f;          // Starting text scale
    f32 FinalScale = 3.5f;          // Target text scale after animation
    f32 dt = 0.0f;          // Delta time snapshot
    f32 timer = 0.0f;          // Elapsed time since page was initialised
}

// UI Elements
TextRenderer GameOverText;            // Renders the "YOU LOSE!" message
Button* restartButton;
Button* menuButton;
// ============================================================================
// Button Callbacks
// ============================================================================

// Restart: Reloads the game from scratch
void static OnFinishRestartClicked()
{
    StateManagerChangeState(STATE_PLAYING);
}

// Main Menu: Returns to the main menu
void static OnFinishMenuClicked()
{
    StateManagerChangeState(STATE_MENU);
}

// ============================================================================
// FinishPage_Load
// ============================================================================
// Loads fonts and creates meshes. Called once on state entry.
// ============================================================================
void FinishPage_Load()
{
    fontPath = AEGfxCreateFont("Assets/buggy-font.ttf", 30);

    Meshes::CreateSquareCenterOriginMesh();
}

// ============================================================================
// FinishPage_Init
// ============================================================================
// Resets animation state and initialises UI. Called every time this state is entered.
// ============================================================================
void FinishPage_Init()
{
    AEGfxSetBackgroundColor(0.1f, 0.1f, 0.15f);

    // Reset animation
    InitialScale = 1.5f;
    timer = 0.0f;

    // "YOU LOSE!" in red
    GameOverText = TextRenderer(fontPath, InitialScale, { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
    GameOverText << "YOU LOSE!";

    restartButton = new Button(fontPath, { -300.0f, -150.0f }, { 300.0f, 75.0f }, OnFinishRestartClicked, { 0.0f, 1.0f, 0.0f, 1.0f }, "Restart");

    menuButton = new Button(fontPath, { 300.0f, -150.0f }, { 300.0f, 75.0f }, OnFinishMenuClicked, { 0.0f, 0.0f, 1.0f, 1.0f }, "Main Menu");

}

// ============================================================================
// FinishPage_Update
// ============================================================================
// Drives the text grow animation. Also handles button logic when enabled.
// ============================================================================
void FinishPage_Update()
{
    // Keep camera centered on the UI
    AEGfxSetCamPosition(0.0f, 0.0f);

    GetMouseWorldPosition(worldMouse.position.x, worldMouse.position.y);

    dt = (f32)AEFrameRateControllerGetFrameTime();
    timer += dt;

    // Grow text toward FinalScale over 6 seconds using lerp-smoothing
    if (timer < 6.0f)
    {
        GameOverText.SetScale(InitialScale += (FinalScale - InitialScale) * 0.6f * dt);
    }

    // --- Restart Button (uncomment to enable) ---
    menuButton->Update(dt);
    restartButton->Update(dt);

    // --- Keyboard shortcut: press R to restart ---
    if (AEInputCheckTriggered(AEVK_R)) OnFinishRestartClicked();
}

// ============================================================================
// FinishPage_Draw
// ============================================================================
// Draws the animated "YOU LOSE!" text (and restart button when enabled).
// ============================================================================
void FinishPage_Draw()
{
    GameOverText.Draw();

    // DrawButton(RestartButton); // Uncomment to render the button
}

// ============================================================================
// FinishPage_Free
// ============================================================================
// Releases runtime text state. Called before the state is exited.
// ============================================================================
void FinishPage_Free()
{
    // Remove buttons from mainPageObj (they self-registered there on construction)
    if (restartButton)
    {
        auto& v = mainPageObj;
        v.erase(std::remove(v.begin(), v.end(), static_cast<GameObject*>(restartButton)), v.end());
        delete restartButton;
        restartButton = nullptr;
    }
    if (menuButton)
    {
        auto& v = mainPageObj;
        v.erase(std::remove(v.begin(), v.end(), static_cast<GameObject*>(menuButton)), v.end());
        delete menuButton;
        menuButton = nullptr;
    }
}

// ============================================================================
// FinishPage_Unload
// ============================================================================
// Destroys font and mesh resources. Called once when leaving the state.
// ============================================================================
void FinishPage_Unload()
{
    AEGfxDestroyFont(fontPath);
    Meshes::FreeMeshes();
}