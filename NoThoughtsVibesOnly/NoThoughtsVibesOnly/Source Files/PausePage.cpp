// ============================================================================
// PausePage.cpp - In-Game Pause Overlay
// ============================================================================
// Owned entirely by PausePage - GamePage just calls these functions.
// isPaused and bgMusic are extern from GamePage.cpp.
//
// CALL ORDER IN GamePage.cpp:
// ----------------------------------------------------------------------------
//   Game_Load()   -> PausePage_Load()
//   Game_Init()   -> PausePage_Init()
//   Game_Draw()   -> if (isPaused) { PausePage_Draw(); PausePage_Update(); }
//   Game_Free()   -> PausePage_Free()
//   Game_Unload() -> PausePage_Unload()
// ============================================================================
#include "pch.hpp"
#include "Player.hpp"
#include "PausePage.hpp"
#include "AEEngine.h"
#include "Util.hpp"
#include "Button.hpp"
#include "Input.hpp"
#include "MenuPage.hpp"

// Shared with GamePage.cpp
extern bool   isPaused;
extern GameObject* pPlayer;

// ============================================================================
// File-scope state
// ============================================================================
namespace
{
    Mouse        worldMouse;
    s8           fontPath{};
    TextRenderer pauseText;
    TextRenderer hintText;
    Button* resumeBtn{ nullptr };
    Button* restartBtn{ nullptr };
    Button* menuBtn{ nullptr };
}

// ============================================================================
// PausePage_Load
// ============================================================================
void PausePage_Load()
{
    fontPath = AEGfxCreateFont("Assets/buggy-font.ttf", 30);
}

// ============================================================================
// PausePage_Init
// ============================================================================
void PausePage_Init()
{
    pauseText = TextRenderer(fontPath, 1.0f, { 0.0f, 200.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
    pauseText << "PAUSED";

    hintText = TextRenderer(fontPath, 0.6f, { 0.0f, -230.0f }, { 0.6f, 0.6f, 0.6f, 1.0f });
    hintText << "ESC to resume  |  TAB to restart";

    resumeBtn = new Button(fontPath, { 0.0f,  50.0f }, { 300.0f, 75.0f },
        []() { isPaused = false;            AudioManager::ResumeMusic();
        Player* p = dynamic_cast<Player*>(pPlayer);
        if (p) p->suppressShootOneFrame = true; },
        { 0.0f, 0.6f, 0.0f, 1.0f }, "RESUME");

    restartBtn = new Button(fontPath, { 0.0f, -50.0f }, { 300.0f, 75.0f },
        []() { isPaused = false; StateManagerChangeState(STATE_RESTART); },
        { 0.8f, 0.5f, 0.0f, 1.0f }, "RESTART");

    menuBtn = new Button(fontPath, { 0.0f, -150.0f }, { 300.0f, 75.0f },
        []() { isPaused = false; StateManagerChangeState(STATE_MENU); },
        { 0.6f, 0.0f, 0.0f, 1.0f }, "MAIN MENU");
}

// ============================================================================
// PausePage_Update
// ============================================================================
// Called from Game_Draw() while isPaused is true.
// Handles keyboard shortcuts and button click detection.
// ============================================================================
void PausePage_Update()
{
    GetMouseWorldPosition(worldMouse.position.x, worldMouse.position.y);

    // Keyboard shortcuts
    if (AEInputCheckTriggered(AEVK_TAB)) { isPaused = false; StateManagerChangeState(STATE_RESTART); return; }
    if (AEInputCheckTriggered(AEVK_Q)) { isPaused = false; StateManagerChangeState(STATE_MENU);    return; }

    const f32 dt = static_cast<f32>(AEFrameRateControllerGetFrameTime());
    resumeBtn->Update(dt);
    restartBtn->Update(dt);
    menuBtn->Update(dt);
}

// ============================================================================
// PausePage_Draw
// ============================================================================
// Dark overlay + PAUSED text + hint text + buttons.
// ============================================================================
void PausePage_Draw()
{
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.7f);

    Transform t;
    t.SetPosition(0.0f, 0.0f);
    t.SetUniformScale(3000.0f);
    t.Apply();
    AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

    pauseText.Draw();
    hintText.Draw();
}

// ============================================================================
// PausePage_Free
// ============================================================================
// Removes buttons from mainPageObj then deletes them.
// Must be called in Game_Free() before mainPageObj is cleared.
// ============================================================================
void PausePage_Free()
{
    if (resumeBtn)
    {
        mainPageObj.erase(std::remove(mainPageObj.begin(), mainPageObj.end(),
            static_cast<GameObject*>(resumeBtn)), mainPageObj.end());
        delete resumeBtn;
        resumeBtn = nullptr;
    }
    if (restartBtn)
    {
        mainPageObj.erase(std::remove(mainPageObj.begin(), mainPageObj.end(),
            static_cast<GameObject*>(restartBtn)), mainPageObj.end());
        delete restartBtn;
        restartBtn = nullptr;
    }
    if (menuBtn)
    {
        mainPageObj.erase(std::remove(mainPageObj.begin(), mainPageObj.end(),
            static_cast<GameObject*>(menuBtn)), mainPageObj.end());
        delete menuBtn;
        menuBtn = nullptr;
    }
}

// ============================================================================
// PausePage_Unload
// ============================================================================
void PausePage_Unload()
{
    AEGfxDestroyFont(fontPath);
}