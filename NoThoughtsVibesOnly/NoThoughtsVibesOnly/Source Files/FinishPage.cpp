// ============================================================================
// FinishPage.cpp - Game Over / Lose Screen
// ============================================================================
// Shown when the player's health reaches zero.
// Displays "YOU LOSE!" text that grows over 6 seconds.
// Two buttons: Restart (goes back to game) and Main Menu.
// Press R to restart via keyboard shortcut.
//
// USAGE:
// ----------------------------------------------------------------------------
//   Entered automatically via StateManagerChangeState(STATE_FINISH).
//   Triggered in Game_Update() when player->health <= 0.
// ============================================================================

#include "pch.hpp"
#include "StarBackground.hpp"
#include "FinishPage.hpp"
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

TextRenderer GameOverText;
Button* restartButton{ nullptr };
Button* menuButton{ nullptr };

// ============================================================================
// Button callbacks
// ============================================================================
static void OnFinishRestartClicked() { StateManagerChangeState(STATE_PLAYING); }
static void OnFinishMenuClicked() { StateManagerChangeState(STATE_MENU); }

// ============================================================================
// FinishPage_Load
// ============================================================================
void FinishPage_Load()
{
    fontPath = AEGfxCreateFont("Assets/buggy-font.ttf", 30);
    Meshes::CreateSquareCenterOriginMesh();
    Meshes::CreateCircleMesh();
    StarBackground::Init();
}

// ============================================================================
// FinishPage_Init
// ============================================================================
void FinishPage_Init()
{
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.02f);

    InitialScale = 1.5f;
    timer = 0.0f;

    GameOverText = TextRenderer(fontPath, InitialScale, { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
    GameOverText << "YOU LOSE!";

    restartButton = new Button(fontPath, { -300.0f, -150.0f }, { 300.0f, 75.0f }, OnFinishRestartClicked, { 0.0f, 1.0f, 0.0f, 1.0f }, "Restart");
    menuButton = new Button(fontPath, { 300.0f, -150.0f }, { 300.0f, 75.0f }, OnFinishMenuClicked, { 0.0f, 0.0f, 1.0f, 1.0f }, "Main Menu");
}

// ============================================================================
// FinishPage_Update
// ============================================================================
// Stars draw before buttons so they appear behind them.
// Text scale grows smoothly toward FinalScale over 6 seconds.
// ============================================================================
void FinishPage_Update()
{
    AEGfxSetCamPosition(0.0f, 0.0f);
    GetMouseWorldPosition(worldMouse.position.x, worldMouse.position.y);

    dt = (f32)AEFrameRateControllerGetFrameTime();
    timer += dt;
    StarBackground::Update(dt);
    StarBackground::DrawBackground();
    StarBackground::Draw();

    if (timer < 6.0f)
        GameOverText.SetScale(InitialScale += (FinalScale - InitialScale) * 0.6f * dt);

    menuButton->Update(dt);
    restartButton->Update(dt);

    if (AEInputCheckTriggered(AEVK_R)) OnFinishRestartClicked();
}

// ============================================================================
// FinishPage_Draw
// ============================================================================
void FinishPage_Draw()
{
    GameOverText.Draw();
}

// ============================================================================
// FinishPage_Free
// ============================================================================
void FinishPage_Free()
{
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
void FinishPage_Unload()
{
    AEGfxDestroyFont(fontPath);
    Meshes::FreeMeshes();
}