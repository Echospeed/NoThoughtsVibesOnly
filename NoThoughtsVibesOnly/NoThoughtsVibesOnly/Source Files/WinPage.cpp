// ============================================================================
// WinPage.cpp - Victory Screen
// ============================================================================
// Shown when the player clears all waves.
// Displays "YOU WIN!" text that grows over 6 seconds.
// Two buttons: Restart (goes back to game) and Main Menu.
// Press R to restart via keyboard shortcut.
//
// USAGE:
// ----------------------------------------------------------------------------
//   Entered automatically via StateManagerChangeState(STATE_WIN).
//   Triggered in Game_Update() when waveSystem.IsLevelComplete() is true.
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
    Mouse worldMouse;
    s8    fontPath{};

    f32 InitialScale = 1.5f; // Scale at start of animation
    f32 FinalScale = 3.5f; // Scale at end of animation
    f32 dt = 0.0f;
    f32 timer = 0.0f;
}

TextRenderer WinText;
Button* rButton; // Restart
Button* mButton; // Main Menu

// ============================================================================
// Button callbacks
// ============================================================================
static void OnWinMenuClicked() { StateManagerChangeState(STATE_MENU); }
static void OnWinRestartClicked() { StateManagerChangeState(STATE_PLAYING); }

// ============================================================================
// WinPage_Load
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
void WinPage_Init()
{
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.02f);

    InitialScale = 1.5f;
    timer = 0.0f;

    WinText = TextRenderer(fontPath, 1.0f, { 0.0f, 250.0f }, { 1.0f, 0.0f, 0.0f });
    WinText << "YOU WIN!";

    rButton = new Button(fontPath, { -300.0f, -150.0f }, { 300.0f, 75.0f }, OnWinRestartClicked, { 0.0f, 1.0f, 0.0f, 1.0f }, "Restart");
    mButton = new Button(fontPath, { 300.0f, -150.0f }, { 300.0f, 75.0f }, OnWinMenuClicked, { 0.0f, 0.0f, 1.0f, 1.0f }, "Main Menu");
}

// ============================================================================
// WinPage_Update
// ============================================================================
// Stars draw before buttons so they appear behind them.
// Text scale grows smoothly toward FinalScale over 6 seconds.
// ============================================================================
void WinPage_Update()
{
    AEGfxSetCamPosition(0.0f, 0.0f);
    GetMouseWorldPosition(worldMouse.position.x, worldMouse.position.y);

    dt = (f32)AEFrameRateControllerGetFrameTime();
    timer += dt;
    StarBackground::Update(dt);
    StarBackground::DrawBackground();
    StarBackground::Draw();

    if (timer < 6.0f)
        InitialScale += (FinalScale - InitialScale) * 0.6f * dt;

    mButton->Update(dt);
    rButton->Update(dt);

    if (AEInputCheckTriggered(AEVK_R)) OnWinRestartClicked();
}

// ============================================================================
// WinPage_Draw
// ============================================================================
void WinPage_Draw()
{
    WinText.SetScale(InitialScale);
    WinText.Draw();
}

// ============================================================================
// WinPage_Free
// ============================================================================
void WinPage_Free()
{
    mainPageObj.erase(std::remove(mainPageObj.begin(), mainPageObj.end(), rButton), mainPageObj.end());
    mainPageObj.erase(std::remove(mainPageObj.begin(), mainPageObj.end(), mButton), mainPageObj.end());

    delete rButton; rButton = nullptr;
    delete mButton; mButton = nullptr;
}

// ============================================================================
// WinPage_Unload
// ============================================================================
void WinPage_Unload()
{
    AEGfxDestroyFont(fontPath);
    Meshes::FreeMeshes();
}