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

    bool        s_ScoreSubmitted = false; // Guard so score is only submitted once per win
    int         s_FinalScore = 0;     // Copied from g_FinalScore on Init
    std::string s_PlayerName = "";    // Name typed by the player
    bool        s_NameSubmitted = false; // True once Enter is pressed
    bool s_NameRejected = false;  // True if Submit() rejected the name as inappropriate

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
    s_ScoreSubmitted = false;
    s_PlayerName = "";
    s_NameSubmitted = false;

    // FIX: Read from g_FinalScore which was saved in Game_Update() right before
    //      the state transition. The old code called waveSystem.GetCurrentWave()
    //      here, but by this point Game_Free() has already run waveSystem.Cleanup()
    //      which resets currentWave to 0 - so the score was always 0.
    s_FinalScore = g_FinalScore;

    // FIX: Removed the old auto-submit with "Player" name that fired on Init().
    //      That was filling the leaderboard with fake "Player" entries before
    //      the player typed anything. Now we wait for Enter to be pressed.

    WinText = TextRenderer(fontPath, 1.0f, { 0.0f, 330.0f }, { 1.0f, 0.0f, 0.0f });
    WinText << "YOU WIN!";

    rButton = new Button(fontPath, { -300.0f, -320.0f }, { 300.0f, 75.0f }, OnWinRestartClicked, { 0.0f, 1.0f, 0.0f, 1.0f }, "Restart");
    mButton = new Button(fontPath, { 300.0f, -320.0f }, { 300.0f, 75.0f }, OnWinMenuClicked, { 0.0f, 0.0f, 1.0f, 1.0f }, "Main Menu");
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

    dt = static_cast<f32>(AEFrameRateControllerGetFrameTime());
    timer += dt;
    StarBackground::Update(dt);
    StarBackground::DrawBackground();
    StarBackground::Draw();

    if (timer < 6.0f)
        InitialScale += (FinalScale - InitialScale) * 0.6f * dt;

    mButton->Update(dt);
    rButton->Update(dt);

    // ==========================================================================
    // --- Name input handling ---
    // ==========================================================================
    if (!s_NameSubmitted)
    {
        // Backspace removes the last character typed
        if (AEInputCheckTriggered(AEVK_BACK) && !s_PlayerName.empty())
            s_PlayerName.pop_back();

        // Enter confirms the name and submits the score to the leaderboard
        if (AEInputCheckTriggered(AEVK_RETURN) && !s_PlayerName.empty())
        {
            // Submit() returns false if the name contains a blocked word
            bool accepted = Leaderboard::Submit(s_PlayerName, s_FinalScore,
                g_CurrentLevel.name,
                g_FinalWaveCount);
            if (accepted)
            {
                s_ScoreSubmitted = true;
                s_NameSubmitted = true;
                s_NameRejected = false;
            }
            else
            {
                // Name was blocked - clear it so player can try again
                s_NameRejected = true;
                s_PlayerName = "";
            }
        }

        // Letter keys A-Z � build up the player's name one character at a time
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

    // TAB restarts after name has been submitted
    if (s_NameSubmitted && AEInputCheckTriggered(AEVK_TAB)) OnWinRestartClicked();
}

// ============================================================================
// WinPage_Draw
// ============================================================================
void WinPage_Draw()
{
    WinText.SetScale(InitialScale);
    WinText.Draw();

    // ==========================================================================
    // Name input box - shown until player submits their name
    // ==========================================================================
    if (!s_NameSubmitted)
    {
        TextRenderer prompt(fontPath, 0.6f, { 0.0f, -170.0f }, { 1.0f, 1.0f, 0.2f, 1.0f });
        prompt << "ENTER YOUR NAME:";
        prompt.Draw();

        // Show what the player has typed so far with a blinking cursor underscore
        std::string display = s_PlayerName + "_";
        TextRenderer nameDisplay(fontPath, 0.8f, { 0.0f, -210.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
        nameDisplay << display;
        nameDisplay.Draw();

        TextRenderer hint(fontPath, 0.45f, { 0.0f, -250.0f }, { 0.6f, 0.6f, 0.6f, 1.0f });
        hint << "Press ENTER to confirm  |  BACKSPACE to delete";
        hint.Draw();

        // Show rejection message if the last submission was blocked
        if (s_NameRejected)
        {
            TextRenderer errMsg(fontPath, 0.65f, { 0.0f, -400.0f }, { 1.0f, 0.15f, 0.15f, 1.0f });
            errMsg << "INVALID NAME - please choose another";
            errMsg.Draw();
        }
    }

    // -------------------------------------------------------------------------
    // Leaderboard display - top 10 scores
    // -------------------------------------------------------------------------
    const auto& entries = Leaderboard::GetEntries();

    TextRenderer lbTitle(fontPath, 0.7f, { 0.0f, 250.0f }, { 1.0f, 1.0f, 0.2f, 1.0f });
    lbTitle << "TOP SCORES";
    lbTitle.Draw();

    for (int i = 0; i < static_cast<int>(entries.size()) && i < 10; ++i)
    {
        const f32 yPos = 200.0f - (i * 35.0f);

        // Highlight this run's score in gold so the player can spot their entry
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