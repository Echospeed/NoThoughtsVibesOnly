// ============================================================================
// LevelSelectPage.cpp - Level Selection Screen
// ============================================================================
// Displays three buttons for level selection:
//   Level 1  : 5 waves, no boss
//   Level 2  : 10 waves, boss on the final wave
//   Endless  : Infinite waves, increasing difficulty
//
// BUTTON SETUP:
// ----------------------------------------------------------------------------
//   Buttons are created in LevelSelect_Init() and deleted in LevelSelect_Free().
//   Each button takes a position, size, colour, label, and a ButtonFunction.
//
// FLOW:
// ----------------------------------------------------------------------------
//   Player selects a level -> sets g_CurrentLevel -> STATE_PLAYING
//   Back button            -> STATE_MENU
// ============================================================================

#include "pch.hpp"
#include "StarBackground.hpp"
#include "LevelSelectPage.hpp"
#include "MenuPage.hpp"
#include "Util.hpp"
#include "AEEngine.h"
#include <vector>
#include <iostream>
#include "Button.hpp"
#include "Input.hpp"
#include "LevelConfig.hpp"

// ============================================================================
// Global object list
// ============================================================================
std::vector<GameObject*> levelSelectPageObj;

// ============================================================================
// Resources
// ============================================================================
static s8    levelSelectFontPath{};
static Mouse levelSelectWorldMouse;

// ============================================================================
// Buttons (heap-allocated, deleted in LevelSelect_Free)
// ============================================================================
static Button* level1Button{ nullptr };
static Button* level2Button{ nullptr };
static Button* endlessButton{ nullptr };
static Button* levelSelectBackButton{ nullptr };



// ============================================================================
// Text renderers
// ============================================================================
static TextRenderer levelSelectTitleText;

// ============================================================================
// Level selection callbacks
// ============================================================================
static void OnLevel1Clicked()
{
    g_CurrentLevel = GetLevelConfig(LevelType::LEVEL_1);
    StateManagerGamePage();
}

static void OnLevel2Clicked()
{
    g_CurrentLevel = GetLevelConfig(LevelType::LEVEL_2);
    StateManagerGamePage();
}

static void OnEndlessClicked()
{
    g_CurrentLevel = GetLevelConfig(LevelType::ENDLESS);
    StateManagerGamePage();
}

static void GoToMainMenu()
{
    StateManagerMenuPage();
}

// ============================================================================
// LevelSelect_Load
// ============================================================================
void LevelSelect_Load()
{
    levelSelectFontPath = AEGfxCreateFont("Assets/buggy-font.ttf", 30);
    Meshes::CreateSquareCenterOriginMesh();
    Meshes::CreateCircleMesh();
    StarBackground::Init();
}

// ============================================================================
// LevelSelect_Init
// ============================================================================
void LevelSelect_Init()
{
    AEGfxSetCamPosition(0.0f, 0.0f);
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.02f);

    level1Button = new Button(levelSelectFontPath, { 0.0f,  100.0f }, { 300.0f, 75.0f },
        OnLevel1Clicked, { 0.2f, 0.5f, 0.8f, 1.0f }, "LEVEL 1");

    level2Button = new Button(levelSelectFontPath, { 0.0f,    0.0f }, { 300.0f, 75.0f },
        OnLevel2Clicked, { 0.3f, 0.6f, 0.3f, 1.0f }, "LEVEL 2");

    endlessButton = new Button(levelSelectFontPath, { 0.0f, -100.0f }, { 300.0f, 75.0f },
        OnEndlessClicked, { 0.8f, 0.4f, 0.0f, 1.0f }, "ENDLESS");

    levelSelectBackButton = new Button(levelSelectFontPath, { 0.0f, -250.0f }, { 200.0f, 75.0f },
        GoToMainMenu, { 0.7f, 0.0f, 0.0f, 1.0f }, "BACK");

    levelSelectTitleText = TextRenderer(levelSelectFontPath, 1.0f, { 0.0f, 250.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
    levelSelectTitleText << "SELECT LEVEL";
}

// ============================================================================
// LevelSelect_Update
// ============================================================================
// Stars draw before buttons so they appear behind them.
// ============================================================================
void LevelSelect_Update()
{
    const f32 dt = static_cast<f32>(AEFrameRateControllerGetFrameTime());
    StarBackground::Update(dt);
    StarBackground::DrawBackground();
    StarBackground::Draw();

    if (level1Button)          level1Button->Update(dt);
    if (level2Button)          level2Button->Update(dt);
    if (endlessButton)         endlessButton->Update(dt);
    if (levelSelectBackButton) levelSelectBackButton->Update(dt);
}

// ============================================================================
// LevelSelect_Draw
// ============================================================================
void LevelSelect_Draw()
{
    levelSelectTitleText.Draw();
}

// ============================================================================
// LevelSelect_Free
// ============================================================================
// Removes buttons from mainPageObj (they self-registered there on construction)
// before deleting them to avoid dangling pointers.
// ============================================================================
void LevelSelect_Free()
{
    mainPageObj.erase(std::remove(mainPageObj.begin(), mainPageObj.end(), level1Button), mainPageObj.end());
    mainPageObj.erase(std::remove(mainPageObj.begin(), mainPageObj.end(), level2Button), mainPageObj.end());
    mainPageObj.erase(std::remove(mainPageObj.begin(), mainPageObj.end(), endlessButton), mainPageObj.end());
    mainPageObj.erase(std::remove(mainPageObj.begin(), mainPageObj.end(), levelSelectBackButton), mainPageObj.end());

    delete level1Button;          level1Button = nullptr;
    delete level2Button;          level2Button = nullptr;
    delete endlessButton;         endlessButton = nullptr;
    delete levelSelectBackButton; levelSelectBackButton = nullptr;

    AudioManager::StopMusic("MenuMusic");
    levelSelectPageObj.clear();
    levelSelectPageObj.shrink_to_fit();
}

// ============================================================================
// LevelSelect_Unload
// ============================================================================
void LevelSelect_Unload()
{
    AEGfxDestroyFont(levelSelectFontPath);
    Meshes::FreeMeshes();
}