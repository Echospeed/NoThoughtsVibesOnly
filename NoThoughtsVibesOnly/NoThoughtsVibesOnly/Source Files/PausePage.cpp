// ============================================================================
// PausePage.cpp - In-Game Pause Overlay
// ============================================================================
// NOTE: This page is NOT currently used as a separate game state.
// Pause is handled directly inside GamePage.cpp using the isPaused flag.
// This file is preserved if the pause screen is promoted to its own state.
//
// TO ENABLE AS A SEPARATE STATE:
// ----------------------------------------------------------------------------
//   1. In Game_Update(), replace the isPaused toggle with:
//          StateManagerChangeState(STATE_PAUSE);
//   2. PauseState is already registered in StateManager::CreateState().
//   3. Call bgMusic->Pause() before entering STATE_PAUSE.
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
}

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

    pauseText = TextRenderer(fontPath, 1.0f, { 0.0f, 300.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
    pauseText << "PAUSED";
}

// ============================================================================
// PausePage_Update
// ============================================================================
// ESC resumes. R restarts. Q goes to menu.
// ============================================================================
void PausePage_Update()
{
    AEGfxSetCamPosition(0.0f, 0.0f);
    GetMouseWorldPosition(worldMouse.position.x, worldMouse.position.y);

    if (AEInputCheckTriggered(AEVK_ESCAPE)) { StateManagerChangeState(STATE_PLAYING); return; }
    if (AEInputCheckTriggered(AEVK_TAB)) { StateManagerChangeState(STATE_RESTART); return; }
    if (AEInputCheckTriggered(AEVK_Q)) { StateManagerChangeState(STATE_MENU);    return; }
}

// ============================================================================
// PausePage_Draw
// ============================================================================
// Draws a dark overlay over the game world, then "PAUSED" text.
// ============================================================================
void PausePage_Draw()
{
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.7f);

    AEMtx33 transform, scale, trans;
    AEMtx33Scale(&scale, 2000.0f, 2000.0f);
    AEMtx33Trans(&trans, 0.0f, 0.0f);
    AEMtx33Concat(&transform, &trans, &scale);
    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

    pauseText.Draw();
}

// ============================================================================
// PausePage_Free
// ============================================================================
void PausePage_Free() {}

// ============================================================================
// PausePage_Unload
// ============================================================================
void PausePage_Unload()
{
    AEGfxDestroyFont(fontPath);
    Meshes::FreeMeshes();
}