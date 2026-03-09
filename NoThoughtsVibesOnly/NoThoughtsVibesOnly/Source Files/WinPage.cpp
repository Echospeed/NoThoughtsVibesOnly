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
#include "StarBackground.hpp"
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
Button* rButton;
Button* mButton;

// ============================================================================
// Button Callbacks
// ============================================================================

// Return to menu after winning
void static OnWinMenuClicked()
{
    StateManagerChangeState(STATE_MENU);
}

// Restart the game from the win screen
void static OnWinRestartClicked()
{
    StateManagerChangeState(STATE_PLAYING);
}

// ============================================================================
// WinPage_Load
// ============================================================================
// Loads fonts and creates mesh resources. Called once on state entry.
// ============================================================================
void WinPage_Load()
{
    fontPath = AEGfxCreateFont("Assets/buggy-font.ttf", 30);
    Meshes::CreateSquareCenterOriginMesh();
    Meshes::CreateCircleMesh();
    StarBackground::Init();
}

// ============================================================================
// WinPage_Init
// ============================================================================
// Resets animation and initialises UI. Called every time this state is entered.
// ============================================================================
void WinPage_Init()
{
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.02f);

    // Reset animation
    InitialScale = 1.5f;
    timer = 0.0f;

    // "YOU WIN!" in red (change colour values to customise)
    WinText = TextRenderer(fontPath, 1.0f, { 0.0f, 250.0f }, { 1.0f, 0.0f, 0.0f });
    WinText << "YOU WIN!";

    rButton = new Button(fontPath, { -300.0f, -150.0f }, { 300.0f, 75.0f }, OnWinRestartClicked, { 0.0f, 1.0f, 0.0f, 1.0f }, "Restart");

    mButton = new Button(fontPath, { 300.0f, -150.0f }, { 300.0f, 75.0f }, OnWinMenuClicked, { 0.0f, 0.0f, 1.0f, 1.0f }, "Main Menu");
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
    StarBackground::Update(dt);

    // Draw background + stars BEFORE buttons so buttons appear on top
    StarBackground::DrawBackground();
    StarBackground::Draw();

    // Smoothly scale text up toward FinalScale over 6 seconds
    if (timer < 6.0f)
    {
        InitialScale += (FinalScale - InitialScale) * 0.6f * dt;
    }

    // --- Restart Button (uncomment to enable) ---
    mButton->Update(dt);
    rButton->Update(dt);

    // --- Keyboard shortcut: press R to restart ---
    if (AEInputCheckTriggered(AEVK_R)) OnWinRestartClicked();
}

// ============================================================================
// WinPage_Draw
// ============================================================================
void WinPage_Draw()
{
    WinText.Draw();
}

// ============================================================================
// WinPage_Free
// ============================================================================
void WinPage_Free()
{
    mainPageObj.erase(std::remove(mainPageObj.begin(), mainPageObj.end(), rButton), mainPageObj.end());
    mainPageObj.erase(std::remove(mainPageObj.begin(), mainPageObj.end(), mButton), mainPageObj.end());

    delete rButton;  rButton = nullptr;
    delete mButton;  mButton = nullptr;
}

// ============================================================================
// WinPage_Unload
// ============================================================================
void WinPage_Unload()
{
    AEGfxDestroyFont(fontPath);
    Meshes::FreeMeshes();
}