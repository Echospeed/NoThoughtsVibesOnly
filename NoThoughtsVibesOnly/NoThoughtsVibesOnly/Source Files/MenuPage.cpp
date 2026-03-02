// ============================================================================
// MenuPage.cpp - Main Menu Implementation
// ============================================================================
// The main menu supports three sub-views: MAIN, CONTROLS, and CREDITS.
// Buttons switch between views via simple state callbacks.
//
// BUTTON SETUP:
// ----------------------------------------------------------------------------
//   Buttons are created in Main_Init() with new Button(...) and deleted in
//   Main_Free(). Each button has:
//     - A position, size, colour, and label text
//     - A ButtonFunction callback (called on left-click)
//
//   Example adding a new button:
//       myButton = new Button(fontPath, {0.0f, 50.0f}, {300.0f, 75.0f},
//                             MyCallbackFunction, {0.5f, 0.5f, 0.0f, 1.0f}, "LABEL");
//
// FLOW:
// ----------------------------------------------------------------------------
//   MENU_MAIN    : Shows Start / Controls / Credits / Quit
//   MENU_CONTROLS: Shows controls info + Back button
//   MENU_CREDITS : Shows credits info + Back button
// ============================================================================

#include "pch.hpp"
#include "MenuPage.hpp"
#include "Util.hpp"
#include "AEEngine.h"
#include <vector>
#include <iostream>
#include "Button.hpp"
#include "Input.hpp"
#include "Audio.hpp"

// ============================================================================
// Menu sub-state
// ============================================================================
enum MenuState
{
    MENU_MAIN,
    MENU_CONTROLS,
    MENU_CREDITS
};

static MenuState currentMenuState = MENU_MAIN;

// ============================================================================
// Global object list (menu objects register themselves here in their ctor)
// ============================================================================
std::vector<GameObject*> mainPageObj;

// ============================================================================
// Resources
// ============================================================================
s8    fontPath{};      // Font handle
Mouse worldMouse;      // Mouse position in world space

// ============================================================================
// Buttons (heap-allocated, cleaned up in Main_Free)
// ============================================================================
Button* startButton{ nullptr };
Button* controlsButton{ nullptr };
Button* creditsButton{ nullptr };
Button* quitButton{ nullptr };
Button* backButton{ nullptr };

// ============================================================================
// Text renderers
// ============================================================================
TextRenderer mainText;      // Title text on main menu
TextRenderer controlsText;  // "CONTROLS" header
TextRenderer creditsText;   // "CREDITS" header
TextRenderer infoText;      // Body text for info pages

// ============================================================================
// Audio (constructed at file scope - loads on game start)
// ============================================================================
//Audio MenuAudio{ "Assets/bouken.mp3", -1, 1.0f, 1.0f, AudioType::MUSIC };

// ============================================================================
// Sub-view navigation callbacks
// ============================================================================
void GoToControls() { currentMenuState = MENU_CONTROLS; }
void GoToCredits() { currentMenuState = MENU_CREDITS; }
void GoToMain() { currentMenuState = MENU_MAIN; }
void GoToLevelSelect() { StateManagerChangeState(STATE_LEVEL_SELECT); }

// ============================================================================
// Main_Load
// ============================================================================
// Loads font and creates mesh resources.
// ============================================================================
void Main_Load()
{
    fontPath = AEGfxCreateFont("Assets/buggy-font.ttf", 30);

    Meshes::CreateSquareCenterOriginMesh();
}

// ============================================================================
// Main_Init
// ============================================================================
// Resets menu state and creates all buttons.
// ============================================================================
void Main_Init()
{
    AEGfxSetCamPosition(0.0f, 0.0f);
    currentMenuState = MENU_MAIN;

    AEGfxSetBackgroundColor(0.1f, 0.1f, 0.15f);

	mainText = TextRenderer(fontPath, { 1.0f, 1.0f }, { 0.0f, 250.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
    mainText << "HUIN!!!!!!!!";

    // --- Main menu buttons ---
    startButton = new Button(fontPath, { 0.0f,  100.0f }, { 300.0f, 75.0f }, GoToLevelSelect, { 0.0f, 0.6f, 0.0f, 1.0f }, "START");
    controlsButton = new Button(fontPath, { 0.0f,    0.0f }, { 300.0f, 75.0f }, GoToControls, { 0.0f, 0.3f, 0.7f, 1.0f }, "CONTROLS");
    creditsButton = new Button(fontPath, { 0.0f, -100.0f }, { 300.0f, 75.0f }, GoToCredits, { 0.5f, 0.0f, 0.5f, 1.0f }, "CREDITS");
    quitButton = new Button(fontPath, { 0.0f, -200.0f }, { 300.0f, 75.0f }, StateManagerQuit, { 0.7f, 0.0f, 0.0f, 1.0f }, "QUIT");

    // --- Back button (shown only in sub-views) ---
    backButton = new Button(fontPath, { 0.0f, -350.0f }, { 200.0f, 75.0f }, GoToMain, { 0.7f, 0.0f, 0.0f, 1.0f }, "BACK");

    // --- Static text for info pages ---
	controlsText = TextRenderer(fontPath, { 1.0f, 1.0f }, { 0.0f, 250.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
	controlsText << "CONTROLS";

	creditsText = TextRenderer(fontPath, { 1.0f, 1.0f }, { 0.0f, 250.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
	creditsText << "CREDITS";
}

// ============================================================================
// Main_Update
// ============================================================================
// Toggles button visibility based on current sub-view, then updates all objects.
// ============================================================================
void Main_Update()
{
    const f32 dt = (f32)AEFrameRateControllerGetFrameTime();

    // Set button visibility based on which sub-view is active
    const bool isMain = (currentMenuState == MENU_MAIN);
    startButton->isActive = isMain;
    controlsButton->isActive = isMain;
    creditsButton->isActive = isMain;
    quitButton->isActive = isMain;
    backButton->isActive = !isMain; // Back only shown in sub-views

    for (auto& obj : mainPageObj)
    {
        if (obj) obj->Update(dt);
    }
}

// ============================================================================
// Main_Draw
// ============================================================================
// Draws the appropriate sub-view content.
// ============================================================================
void Main_Draw()
{
    if (currentMenuState == MENU_MAIN)
    {
		mainText.Draw();
    }
    else if (currentMenuState == MENU_CONTROLS)
    {
		controlsText.Draw();

        infoText = TextRenderer(fontPath, { 1.0f, 1.0f }, { 0.0f, 100.0f }, {1.0f, 1.0f, 1.0f }); // Reset text renderer to clear previous text
        infoText << "W, A, S, D  -  Move";
		infoText.Draw();

        infoText = TextRenderer(fontPath, { 1.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f });
        infoText << "Left Click  -  Shoot";
		infoText.Draw();

        infoText = TextRenderer(fontPath, { 1.0f, 1.0f }, { 0.0f, -100.0f }, { 1.0f, 1.0f, 1.0f });
		infoText << "ESC  -  Pause  |  R  -  Restart  |  Q  -  Menu";
		infoText.Draw();
    }
    else if (currentMenuState == MENU_CREDITS)
    {
		creditsText.Draw();

		infoText = TextRenderer(fontPath, { 1.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }); // Reset text renderer to clear previous text
        infoText << "Created By:\n\n"
			<< "   - Huin Lee\n   - Justin Lee\n   - Kevin Lin\n   - Daniel Kim\n   - Eric Lee";
		infoText.Draw();

        infoText = TextRenderer(fontPath, { 1.0f, 1.0f }, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f });
        infoText << "No Thoughts, Vibes Only";
		infoText.Draw();

        infoText = TextRenderer(fontPath, { 1.0f, 1.0f }, { 0.0f, -100.0f }, { 1.0f, 1.0f, 1.0f });
        infoText << "DigiPen Institute of Technology";
		infoText.Draw();
    }
}

// ============================================================================
// Main_Free
// ============================================================================
// Deletes buttons and text renderers. Clears the object list.
// ============================================================================
void Main_Free()
{

    delete startButton;    startButton = nullptr;
    delete controlsButton; controlsButton = nullptr;
    delete creditsButton;  creditsButton = nullptr;
    delete quitButton;     quitButton = nullptr;
    delete backButton;     backButton = nullptr;

    mainPageObj.clear();
    mainPageObj.shrink_to_fit();
}

// ============================================================================
// Main_Unload
// ============================================================================
void Main_Unload()
{
    AEGfxDestroyFont(fontPath);
    Meshes::FreeMeshes();
}