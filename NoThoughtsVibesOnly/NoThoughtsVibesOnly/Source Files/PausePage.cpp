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

extern bool        isPaused;
extern GameObject* pPlayer;

namespace
{
    Mouse        worldMouse;
    s8           fontPath{};
    TextRenderer pauseText;
    TextRenderer hintText;
    Button* resumeBtn{ nullptr };
    Button* restartBtn{ nullptr };
    Button* menuBtn{ nullptr };

    // ?? Confirmation dialog state ??
    bool               s_ShowConfirm = false;
    std::function<void()> s_ConfirmAction = nullptr;
    Button* confirmYesBtn{ nullptr };
    Button* confirmNoBtn{ nullptr };

    void ShowConfirm(std::function<void()> action)
    {
        s_ShowConfirm = true;
        s_ConfirmAction = action;
    }
}

void PausePage_Load()
{
    fontPath = AEGfxCreateFont("Assets/buggy-font.ttf", 30);
}

void PausePage_Init()
{
    s_ShowConfirm = false;
    s_ConfirmAction = nullptr;

    pauseText = TextRenderer(fontPath, 1.0f, { 0.0f, 200.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
    pauseText << "PAUSED";

    hintText = TextRenderer(fontPath, 0.6f, { 0.0f, -230.0f }, { 0.6f, 0.6f, 0.6f, 1.0f });
    hintText << "ESC to resume  |  TAB to restart";

    resumeBtn = new Button(fontPath, { 0.0f, 50.0f }, { 300.0f, 75.0f },
        []() {
            isPaused = false;
            AudioManager::ResumeMusic();
            Player* p = dynamic_cast<Player*>(pPlayer);
            if (p) p->suppressShootOneFrame = true;
        },
        { 0.0f, 0.6f, 0.0f, 1.0f }, "RESUME");

    // Destructive actions now show confirmation first
    restartBtn = new Button(fontPath, { 0.0f, -50.0f }, { 300.0f, 75.0f },
        []() { ShowConfirm([]() { isPaused = false; StateManagerChangeState(STATE_RESTART); }); },
        { 0.8f, 0.5f, 0.0f, 1.0f }, "RESTART");

    menuBtn = new Button(fontPath, { 0.0f, -150.0f }, { 300.0f, 75.0f },
        []() { ShowConfirm([]() { isPaused = false; StateManagerChangeState(STATE_MENU); }); },
        { 0.6f, 0.0f, 0.0f, 1.0f }, "MAIN MENU");

    // Yes / No buttons for confirm dialog
    confirmYesBtn = new Button(fontPath, { -100.0f, -50.0f }, { 150.0f, 60.0f },
        []() { if (s_ConfirmAction) s_ConfirmAction(); s_ShowConfirm = false; },
        { 0.0f, 0.6f, 0.0f, 1.0f }, "YES");

    confirmNoBtn = new Button(fontPath, { 100.0f, -50.0f }, { 150.0f, 60.0f },
        []() { s_ShowConfirm = false; s_ConfirmAction = nullptr; },
        { 0.6f, 0.0f, 0.0f, 1.0f }, "NO");
}

void PausePage_Update()
{
    GetMouseWorldPosition(worldMouse.position.x, worldMouse.position.y);
    const f32 dt = static_cast<float>(AEFrameRateControllerGetFrameTime());

    if (s_ShowConfirm)
    {
        // Block all other input while confirming
        confirmYesBtn->Update(dt);
        confirmNoBtn->Update(dt);
        return;
    }

    // Keyboard shortcuts only when no confirm dialog
    if (AEInputCheckTriggered(AEVK_TAB))
    {
        ShowConfirm([]() { isPaused = false; StateManagerChangeState(STATE_RESTART); });
        return;
    }
    if (AEInputCheckTriggered(AEVK_Q))
    {
        ShowConfirm([]() { isPaused = false; StateManagerChangeState(STATE_MENU); });
        return;
    }

    resumeBtn->Update(dt);
    restartBtn->Update(dt);
    menuBtn->Update(dt);
}

void PausePage_Draw()
{
    // Dark overlay
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.7f);
    Transform t;
    t.SetPosition(0.0f, 0.0f);
    t.SetUniformScale(3000.0f);
    t.Apply();
    AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

    if (s_ShowConfirm)
    {
        // Extra dark overlay for confirm dialog
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.5f);
        t.SetUniformScale(3000.0f);
        t.Apply();
        AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

        TextRenderer confirmTitle(fontPath, 1.0f, { 0.0f, 80.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
        confirmTitle << "ARE YOU SURE?";
        confirmTitle.Draw();

        TextRenderer confirmHint(fontPath, 0.6f, { 0.0f, 20.0f }, { 0.7f, 0.7f, 0.7f, 1.0f });
        confirmHint << "This action cannot be undone.";
        confirmHint.Draw();

        return; // buttons draw themselves in Update
    }

    pauseText.Draw();
    hintText.Draw();
}

void PausePage_Free()
{
    auto eraseFrom = [](Button* b)
        {
            if (!b) return;
            mainPageObj.erase(std::remove(mainPageObj.begin(), mainPageObj.end(),
                static_cast<GameObject*>(b)), mainPageObj.end());
            delete b;
        };

    eraseFrom(resumeBtn);   resumeBtn = nullptr;
    eraseFrom(restartBtn);  restartBtn = nullptr;
    eraseFrom(menuBtn);     menuBtn = nullptr;
    eraseFrom(confirmYesBtn); confirmYesBtn = nullptr;
    eraseFrom(confirmNoBtn);  confirmNoBtn = nullptr;
}

void PausePage_Unload()
{
    AEGfxDestroyFont(fontPath);
}