// ============================================================================
// LevelSelectPage.cpp - Level Selection Implementation
// ============================================================================
// Displays three buttons for level selection:
//   - Level 1 (placeholder for future implementation)
//   - Level 2 (placeholder for future implementation)
//   - Endless (links to current GamePage)
//
// BUTTON SETUP:
// ----------------------------------------------------------------------------
//   Buttons are created in LevelSelect_Init() with new Button(...) and 
//   deleted in LevelSelect_Free(). Each button has:
//     - A position, size, colour, and label text
//     - A ButtonFunction callback (called on left-click)
//
// FLOW:
// ----------------------------------------------------------------------------
//   Player selects a level -> Transitions to STATE_PLAYING
//   Back button -> Returns to STATE_MENU
// ============================================================================

#include "pch.hpp"
#include "LevelSelectPage.hpp"
#include "MenuPage.hpp"
#include "Util.hpp"
#include "AEEngine.h"
#include <vector>
#include <iostream>
#include "Button.hpp"
#include "Input.hpp"

// ============================================================================
// Global object list (level select objects register themselves here)
// ============================================================================
std::vector<GameObject*> levelSelectPageObj;

// ============================================================================
// Resources
// ============================================================================
static s8    levelSelectFontPath{};      // Font handle
static Mouse levelSelectWorldMouse;       // Mouse position in world space

// ============================================================================
// Buttons (heap-allocated, cleaned up in LevelSelect_Free)
// ============================================================================
static Button* level1Button{ nullptr };
static Button* level2Button{ nullptr };
static Button* endlessButton{ nullptr };
static Button* levelSelectBackButton{ nullptr };

// ============================================================================
// Text renderers
// ============================================================================
static TextRenderer levelSelectTitleText;  // Title text "SELECT LEVEL"

// ============================================================================
// Level Selection Callbacks
// ============================================================================

// Level 1 - placeholder for future implementation
void OnLevel1Clicked()
{
    // TODO: Set level 1 configuration
    StateManagerGamePage();  // For now, goes to the game
}

// Level 2 - placeholder for future implementation
void OnLevel2Clicked()
{
    // TODO: Set level 2 configuration
    StateManagerGamePage();  // For now, goes to the game
}

// Endless mode - links to current GamePage
void OnEndlessClicked()
{
    StateManagerGamePage();
}

// Back to main menu
void GoToMainMenu()
{
    StateManagerMenuPage();
}

// ============================================================================
// LevelSelect_Load
// ============================================================================
// Loads font and creates mesh resources.
// ============================================================================
void LevelSelect_Load()
{
    levelSelectFontPath = AEGfxCreateFont("Assets/buggy-font.ttf", 30);
    LoadTextRenderer(levelSelectTitleText, levelSelectFontPath);
    Meshes::CreateSquareCenterOriginMesh();
}

// ============================================================================
// LevelSelect_Init
// ============================================================================
// Creates all buttons for level selection.
// ============================================================================
void LevelSelect_Init()
{
    AEGfxSetCamPosition(0.0f, 0.0f);
    AEGfxSetBackgroundColor(0.1f, 0.1f, 0.15f);

    levelSelectTitleText.SetText("SELECT LEVEL");

    // --- Level selection buttons ---
    level1Button = new Button(levelSelectFontPath, { 0.0f,  100.0f }, { 300.0f, 75.0f }, 
                              OnLevel1Clicked, { 0.2f, 0.5f, 0.8f, 1.0f }, "LEVEL 1");
    
    level2Button = new Button(levelSelectFontPath, { 0.0f,    0.0f }, { 300.0f, 75.0f }, 
                              OnLevel2Clicked, { 0.3f, 0.6f, 0.3f, 1.0f }, "LEVEL 2");
    
    endlessButton = new Button(levelSelectFontPath, { 0.0f, -100.0f }, { 300.0f, 75.0f }, 
                               OnEndlessClicked, { 0.8f, 0.4f, 0.0f, 1.0f }, "ENDLESS");

    // --- Back button ---
    levelSelectBackButton = new Button(levelSelectFontPath, { 0.0f, -250.0f }, { 200.0f, 75.0f }, 
                                       GoToMainMenu, { 0.7f, 0.0f, 0.0f, 1.0f }, "BACK");

    // --- Title text ---
    InitTextRenderer(levelSelectTitleText, "SELECT LEVEL", { 1.0f, 1.0f }, 1.0f, 1.0f, 1.0f);
}

// ============================================================================
// LevelSelect_Update
// ============================================================================
// Updates all buttons manually.
// ============================================================================
void LevelSelect_Update()
{
    const f32 dt = (f32)AEFrameRateControllerGetFrameTime();

    // Manually update each button (they register to mainPageObj but we handle them here)
    if (level1Button) level1Button->Update(dt);
    if (level2Button) level2Button->Update(dt);
    if (endlessButton) endlessButton->Update(dt);
    if (levelSelectBackButton) levelSelectBackButton->Update(dt);
}

// ============================================================================
// LevelSelect_Draw
// ============================================================================
// Draws the level selection UI.
// ============================================================================
void LevelSelect_Draw()
{
    DrawTextRenderer(levelSelectTitleText, { 0.0f, 250.0f }, 1.5f);
}

// ============================================================================
// LevelSelect_Free
// ============================================================================
// Deletes buttons and text renderers. Clears the object list.
// Note: Buttons register to mainPageObj (since Button uses STATE_MENU),
// so we must clear that list to avoid dangling pointers.
// ============================================================================
void LevelSelect_Free()
{
    FreeTextRenderer(levelSelectTitleText);

    delete level1Button;           level1Button = nullptr;
    delete level2Button;           level2Button = nullptr;
    delete endlessButton;          endlessButton = nullptr;
    delete levelSelectBackButton;  levelSelectBackButton = nullptr;

    // Buttons registered themselves to mainPageObj, so clear it to avoid dangling pointers
    mainPageObj.clear();
    mainPageObj.shrink_to_fit();

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
