// Author: Stanley Lu

// ============================================================================
// WinPage.cpp - Victory Screen
// ============================================================================
// Shown when the player clears all waves.
// Displays "YOU WIN!" text that grows over 6 seconds.
// Two buttons: Restart (goes back to game) and Main Menu.
// Press TAB to restart via keyboard shortcut.
// Player can enter their name and submit to the leaderboard.
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
#include "Leaderboard.hpp"
#include "LevelConfig.hpp"
#include "WaveSystem.hpp"
#include "GamePage.hpp"

// ============================================================================
// File-scope state
// ============================================================================
namespace
{
    Mouse worldMouse;
    s8    fontPath{};

    f32 InitialScale = 1.5f;
    f32 FinalScale = 3.5f;
    f32 dt = 0.0f;
    f32 timer = 0.0f;

    bool        s_ScoreSubmitted = false;
    int         s_FinalScore = 0;
    std::string s_PlayerName = "";
    bool        s_NameSubmitted = false;
    bool        s_NameRejected = false;

    // Confirmation dialog — managed manually, NOT in mainPageObj
    bool                  s_ShowConfirm = false;
    std::function<void()> s_ConfirmAction = nullptr;
    Button* s_ConfirmYesBtn{ nullptr };
    Button* s_ConfirmNoBtn{ nullptr };

    void ShowConfirm(std::function<void()> action)
    {
        s_ShowConfirm = true;
        s_ConfirmAction = action;
    }
}

TextRenderer WinText;
Button* rButton{ nullptr };
Button* mButton{ nullptr };

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
    s_ScoreSubmitted = false;
    s_PlayerName = "";
    s_NameSubmitted = false;
    s_NameRejected = false;
    s_ShowConfirm = false;
    s_ConfirmAction = nullptr;

    s_FinalScore = g_FinalScore;

    WinText = TextRenderer(fontPath, 1.0f, { 0.0f, 330.0f }, { 1.0f, 0.0f, 0.0f });
    WinText << "YOU WIN!";

    rButton = new Button(fontPath, { -300.0f, -320.0f }, { 300.0f, 75.0f },
        []() { ShowConfirm([]() { OnWinRestartClicked(); }); },
        { 0.0f, 1.0f, 0.0f, 1.0f }, "Restart");

    mButton = new Button(fontPath, { 300.0f, -320.0f }, { 300.0f, 75.0f },
        []() { ShowConfirm([]() { OnWinMenuClicked(); }); },
        { 0.0f, 0.0f, 1.0f, 1.0f }, "Main Menu");

    // Create confirm buttons then remove from mainPageObj
    // so they don't auto-draw or get double-deleted
    s_ConfirmYesBtn = new Button(fontPath, { -100.0f, -50.0f }, { 150.0f, 60.0f },
        []() { if (s_ConfirmAction) s_ConfirmAction(); s_ShowConfirm = false; },
        { 0.0f, 0.6f, 0.0f, 1.0f }, "YES");

    s_ConfirmNoBtn = new Button(fontPath, { 100.0f, -50.0f }, { 150.0f, 60.0f },
        []() { s_ShowConfirm = false; s_ConfirmAction = nullptr; },
        { 0.6f, 0.0f, 0.0f, 1.0f }, "NO");

    mainPageObj.erase(std::remove(mainPageObj.begin(), mainPageObj.end(),
        static_cast<GameObject*>(s_ConfirmYesBtn)), mainPageObj.end());
    mainPageObj.erase(std::remove(mainPageObj.begin(), mainPageObj.end(),
        static_cast<GameObject*>(s_ConfirmNoBtn)), mainPageObj.end());
}

// ============================================================================
// WinPage_Update
// ============================================================================
void WinPage_Update()
{
    // Confirmation dialog — block everything else
    if (s_ShowConfirm)
    {
        const f32 cdt = static_cast<float>(AEFrameRateControllerGetFrameTime());
        s_ConfirmYesBtn->Update(cdt);
        s_ConfirmNoBtn->Update(cdt);
        return;
    }

    AEGfxSetCamPosition(0.0f, 0.0f);
    GetMouseWorldPosition(worldMouse.position.x, worldMouse.position.y);

    dt = static_cast<float>(AEFrameRateControllerGetFrameTime());
    timer += dt;
    StarBackground::Update(dt);
    StarBackground::DrawBackground();
    StarBackground::Draw();

    if (timer < 6.0f)
        InitialScale += (FinalScale - InitialScale) * 0.6f * dt;

    mButton->Update(dt);
    rButton->Update(dt);

    if (!s_NameSubmitted)
    {
        if (AEInputCheckTriggered(AEVK_BACK) && !s_PlayerName.empty())
            s_PlayerName.pop_back();

        if (AEInputCheckTriggered(AEVK_RETURN) && !s_PlayerName.empty())
        {
            bool accepted = Leaderboard::Submit(s_PlayerName, s_FinalScore,
                g_CurrentLevel.name, g_FinalWaveCount);
            if (accepted)
            {
                s_ScoreSubmitted = true;
                s_NameSubmitted = true;
                s_NameRejected = false;
            }
            else
            {
                s_NameRejected = true;
                s_PlayerName = "";
            }
        }

        static const struct { int vk; char ch; } keys[] = {
            {AEVK_A,'A'},{AEVK_B,'B'},{AEVK_C,'C'},{AEVK_D,'D'},{AEVK_E,'E'},
            {AEVK_F,'F'},{AEVK_G,'G'},{AEVK_H,'H'},{AEVK_I,'I'},{AEVK_J,'J'},
            {AEVK_K,'K'},{AEVK_L,'L'},{AEVK_M,'M'},{AEVK_N,'N'},{AEVK_O,'O'},
            {AEVK_P,'P'},{AEVK_Q,'Q'},{AEVK_R,'R'},{AEVK_S,'S'},{AEVK_T,'T'},
            {AEVK_U,'U'},{AEVK_V,'V'},{AEVK_W,'W'},{AEVK_X,'X'},{AEVK_Y,'Y'},
            {AEVK_Z,'Z'}
        };
        for (auto& k : keys)
            if (AEInputCheckTriggered(static_cast<char>(k.vk)) && s_PlayerName.size() < 12)
                s_PlayerName += k.ch;
    }

    if (s_NameSubmitted && AEInputCheckTriggered(AEVK_TAB)) OnWinRestartClicked();
}

// ============================================================================
// WinPage_Draw
// ============================================================================
void WinPage_Draw()
{
    if (s_ShowConfirm)
    {
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.0f);
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.85f);
        Transform t;
        t.SetPosition(0.0f, 0.0f);
        t.SetUniformScale(3000.0f);
        t.Apply();
        AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

        TextRenderer confirmTitle(fontPath, 1.0f, { 0.0f, 80.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
        confirmTitle << "ARE YOU SURE?";
        confirmTitle.Draw();

        TextRenderer confirmHint(fontPath, 0.6f, { 0.0f, 20.0f }, { 0.7f, 0.7f, 0.7f, 1.0f });
        confirmHint << "This action cannot be undone.";
        confirmHint.Draw();

        s_ConfirmYesBtn->Draw();
        s_ConfirmNoBtn->Draw();
        s_ConfirmYesBtn->textRenderer.Draw();
        s_ConfirmNoBtn->textRenderer.Draw();
        return;
    }

    WinText.SetScale(InitialScale);
    WinText.Draw();

    if (!s_NameSubmitted)
    {
        TextRenderer prompt(fontPath, 0.6f, { 0.0f, -170.0f }, { 1.0f, 1.0f, 0.2f, 1.0f });
        prompt << "ENTER YOUR NAME:";
        prompt.Draw();

        std::string display = s_PlayerName + "_";
        TextRenderer nameDisplay(fontPath, 0.8f, { 0.0f, -210.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
        nameDisplay << display;
        nameDisplay.Draw();

        TextRenderer hint(fontPath, 0.45f, { 0.0f, -250.0f }, { 0.6f, 0.6f, 0.6f, 1.0f });
        hint << "Press ENTER to confirm  |  BACKSPACE to delete";
        hint.Draw();

        if (s_NameRejected)
        {
            TextRenderer errMsg(fontPath, 0.65f, { 0.0f, -400.0f }, { 1.0f, 0.15f, 0.15f, 1.0f });
            errMsg << "INVALID NAME - please choose another";
            errMsg.Draw();
        }
    }

    const auto& entries = Leaderboard::GetEntries();

    TextRenderer lbTitle(fontPath, 0.7f, { 0.0f, 250.0f }, { 1.0f, 1.0f, 0.2f, 1.0f });
    lbTitle << "TOP SCORES";
    lbTitle.Draw();

    for (int i = 0; i < static_cast<int>(entries.size()) && i < 10; ++i)
    {
        const f32  yPos = 200.0f - (i * 35.0f);
        const bool isNew = (entries[i].score == s_FinalScore);
        Colour col = isNew ? Colour{ 1.0f, 0.9f, 0.1f, 1.0f } : Colour{ 0.8f, 0.8f, 0.8f, 1.0f };

        TextRenderer row(fontPath, 0.55f, { 0.0f, yPos }, col);
        row << (i + 1) << ". " << entries[i].name
            << "  " << entries[i].score << " pts"
            << "  [" << entries[i].level << "]";
        row.Draw();
    }
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

    // Confirm buttons were removed from mainPageObj — delete separately
    if (s_ConfirmYesBtn) { delete s_ConfirmYesBtn; s_ConfirmYesBtn = nullptr; }
    if (s_ConfirmNoBtn) { delete s_ConfirmNoBtn;  s_ConfirmNoBtn = nullptr; }
    s_ShowConfirm = false;
}

// ============================================================================
// WinPage_Unload
// ============================================================================
void WinPage_Unload()
{
    AEGfxDestroyFont(fontPath);
    Meshes::FreeMeshes();
}