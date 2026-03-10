// ============================================================================
// FinishPage.cpp - Game Over / Lose Screen
// ============================================================================
// Shown when the player's health reaches zero.
// Displays "YOU LOSE!" text that grows over 6 seconds.
// Two buttons: Restart (goes back to game) and Main Menu.
// Press TAB to restart via keyboard shortcut.
// Player can enter their name and submit to the leaderboard.
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
#include "Leaderboard.hpp"
#include "LevelConfig.hpp"
#include "WaveSystem.hpp"

// ============================================================================
// File-scope state
// ============================================================================
namespace
{
    Mouse worldMouse;
    s8    fontPath{};

    f32 InitialScale = 1.5f; // Scale at start of animation
    f32 FinalScale = 3.5f;   // Scale at end of animation
    f32 dt = 0.0f;
    f32 timer = 0.0f;

    bool s_ScoreSubmitted = false; // Guard so score is only submitted once per loss
    int  s_FinalScore = 0;         // Calculated on FinishPage_Init
    std::string s_PlayerName = ""; // Name typed by the player
    bool s_NameSubmitted = false;  // True once Enter is pressed
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
    s_ScoreSubmitted = false;
    s_PlayerName = "";
    s_NameSubmitted = false;

    // Calculate final score: 100 pts per wave cleared
    s_FinalScore = (int)g_CurrentLevel.numWaves * 100;

    // Submit default score (only once per lose screen entry)
    Leaderboard::Submit("Player", s_FinalScore, g_CurrentLevel.name, (int)g_CurrentLevel.numWaves);
    s_ScoreSubmitted = true;

    GameOverText = TextRenderer(fontPath, InitialScale, { 0.0f, 330.0f }, { 1.0f, 0.0f, 0.0f, 1.0f });
    GameOverText << "YOU LOSE!";

    restartButton = new Button(fontPath, { -300.0f, -320.0f }, { 300.0f, 75.0f }, OnFinishRestartClicked, { 0.0f, 1.0f, 0.0f, 1.0f }, "Restart");
    menuButton = new Button(fontPath, { 300.0f, -320.0f }, { 300.0f, 75.0f }, OnFinishMenuClicked, { 0.0f, 0.0f, 1.0f, 1.0f }, "Main Menu");
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

    // ==========================================================================
    // --- Name input handling ---
    // ==========================================================================
    if (!s_NameSubmitted)
    {
        // Backspace
        if (AEInputCheckTriggered(AEVK_BACK) && !s_PlayerName.empty())
            s_PlayerName.pop_back();

        // Enter to confirm
        if (AEInputCheckTriggered(AEVK_RETURN) && !s_PlayerName.empty())
        {
            Leaderboard::Submit(s_PlayerName, s_FinalScore, g_CurrentLevel.name, (int)g_CurrentLevel.numWaves);
            s_ScoreSubmitted = true;
            s_NameSubmitted = true;
        }

        // Letter keys A-Z
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

    if (s_NameSubmitted && AEInputCheckTriggered(AEVK_TAB)) OnFinishRestartClicked();
}

// ============================================================================
// FinishPage_Draw
// ============================================================================
void FinishPage_Draw()
{
    GameOverText.Draw();

    // ==========================================================================
    // Name input box - shown until player submits their name
    // ==========================================================================
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
    }

    // -------------------------------------------------------------------------
    // Leaderboard display - top 10 scores
    // -------------------------------------------------------------------------
    const auto& entries = Leaderboard::GetEntries();

    TextRenderer lbTitle(fontPath, 0.7f, { 0.0f, 250.0f }, { 1.0f, 1.0f, 0.2f, 1.0f });
    lbTitle << "TOP SCORES";
    lbTitle.Draw();

    for (int i = 0; i < (int)entries.size() && i < 10; ++i)
    {
        const f32 yPos = 200.0f - (i * 35.0f);

        // Highlight the current run's score in gold
        const bool isNew = (entries[i].score == s_FinalScore);
        Colour col = isNew ? Colour{ 1.0f, 0.9f, 0.1f, 1.0f }
        : Colour{ 0.8f, 0.8f, 0.8f, 1.0f };

        TextRenderer row(fontPath, 0.55f, { 0.0f, yPos }, col);
        row << (i + 1) << ". " << entries[i].name
            << "  " << entries[i].score << " pts"
            << "  [" << entries[i].level << "]";
        row.Draw();
    }
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