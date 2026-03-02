// ============================================================================
// PausePage.cpp - In-Game Pause Overlay
// ============================================================================
// NOTE: This page is NOT currently used as a separate game state.
// The pause overlay is handled directly inside Game_Draw() / Game_Update()
// using the isPaused flag in GamePage.cpp.
//
// This file is preserved for future use if the pause screen is promoted to
// its own state (STATE_PAUSE). The ESC key in GamePage.cpp handles pause
// toggling; the pause screen shows:
//   "PAUSED" text
//   "ESC-Resume  R-Restart  Q-Menu" hint
//
// TO ENABLE AS A SEPARATE STATE:
// ----------------------------------------------------------------------------
//   1. In GamePage::Game_Update, replace isPaused toggle with:
//        StateManagerChangeState(STATE_PAUSE);
//   2. Add PauseState to StateManager::CreateState() (already present).
//   3. Ensure bgMusic->Pause() is called before entering STATE_PAUSE.
//
// BUTTON CALLBACKS (ready to use when uncommented):
// ----------------------------------------------------------------------------
//   OnResumeClicked()   -> STATE_PLAYING  (return to game)
//   OnRestartClicked()  -> STATE_RESTART  (restart game)
//   OnMainMenuClicked() -> STATE_MENU     (go to menu)
// ============================================================================

#include "pch.hpp"
#include "PausePage.hpp"
#include "AEEngine.h"
#include "Util.hpp"
#include "Button.hpp"
#include "Input.hpp"

// ============================================================================
// File-scope state
// ============================================================================
namespace
{
    Mouse worldMouse;
    s8    fontPath{};

    TextRenderer pauseText;
    TextRenderer escHintText;
}

// ============================================================================
// Button Callbacks
// ============================================================================

void OnResumeClicked() { StateManagerChangeState(STATE_PLAYING); }
void OnRestartClicked() { StateManagerChangeState(STATE_RESTART); }
void OnMainMenuClicked() { StateManagerChangeState(STATE_MENU); }

// ============================================================================
// PausePage_Load
// ============================================================================
void PausePage_Load()
{
    fontPath = AEGfxCreateFont("Assets/buggy-font.ttf", 30);

    Meshes::CreateSquareCenterOriginMesh();
}

// ============================================================================
// PausePage_Init
// ============================================================================
void PausePage_Init()
{
    AEGfxSetCamPosition(0.0f, 0.0f);

	pauseText = TextRenderer(fontPath, { 1.0f, 1.0f }, { 0.0f, 300.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
	pauseText << "PAUSED";

	escHintText = TextRenderer(fontPath, { 0.8f, 0.8f }, { 0.0f, -250.0f }, { 0.7f, 0.7f, 0.7f, 1.0f });
	escHintText << "Press ESC to Resume";
}

// ============================================================================
// PausePage_Update
// ============================================================================
// ESC returns to the game. R restarts. Q goes to menu.
// ============================================================================
void PausePage_Update()
{
    AEGfxSetCamPosition(0.0f, 0.0f);
    GetMouseWorldPosition(worldMouse.position.x, worldMouse.position.y);

    if (AEInputCheckTriggered(AEVK_ESCAPE)) { StateManagerChangeState(STATE_PLAYING); return; }
    if (AEInputCheckTriggered(AEVK_R)) { StateManagerChangeState(STATE_RESTART); return; }
    if (AEInputCheckTriggered(AEVK_Q)) { StateManagerChangeState(STATE_MENU);    return; }

    // --- Button handling (uncomment when buttons are re-enabled) ---
    // resumeButton.isHovered    = isOverlapping(resumeButton.collider,    worldMouse);
    // restartButton.isHovered   = isOverlapping(restartButton.collider,   worldMouse);
    // mainMenuButton.isHovered  = isOverlapping(mainMenuButton.collider,  worldMouse);
    // if (AEInputCheckTriggered(AEVK_LBUTTON))
    // {
    //     if (resumeButton.isHovered)   resumeButton.onClick();
    //     if (restartButton.isHovered)  restartButton.onClick();
    //     if (mainMenuButton.isHovered) mainMenuButton.onClick();
    // }
}

// ============================================================================
// PausePage_Draw
// ============================================================================
// Draws a dark overlay over the game world, then "PAUSED" text and hint.
// ============================================================================
void PausePage_Draw()
{
    // Semi-transparent dark overlay
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.7f);

    AEMtx33 transform, scale, trans;
    AEMtx33Scale(&scale, 2000.0f, 2000.0f);
    AEMtx33Trans(&trans, 0.0f, 0.0f);
    AEMtx33Concat(&transform, &trans, &scale);
    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

    // "PAUSED" header
    pauseText.Draw();

    // Keyboard hint
	escHintText.Draw();
    // DrawButton(resumeButton);   // Uncomment to enable buttons
    // DrawButton(restartButton);
    // DrawButton(mainMenuButton);
}

// ============================================================================
// PausePage_Free
// ============================================================================
void PausePage_Free()
{

}

// ============================================================================
// PausePage_Unload
// ============================================================================
void PausePage_Unload()
{
    AEGfxDestroyFont(fontPath);
    Meshes::FreeMeshes();
}