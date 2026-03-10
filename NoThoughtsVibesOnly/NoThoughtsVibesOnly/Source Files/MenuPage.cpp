// ============================================================================
// MenuPage.cpp - Main Menu Implementation
// ============================================================================
// Supports three sub-views switched via simple state callbacks:
//
//   MENU_MAIN     : Start / Controls / Credits / Quit
//   MENU_CONTROLS : Controls info + Back button
//   MENU_CREDITS  : Credits image + Back button
//
// BUTTON SETUP:
// ----------------------------------------------------------------------------
//   Buttons are created in Main_Init() with new Button(...) and deleted in
//   Main_Free(). Each button takes a position, size, colour, label, and a
//   ButtonFunction callback (fired on left-click).
//
//   Example adding a new button:
//       myButton = new Button(fontPath, {0.0f, 50.0f}, {300.0f, 75.0f},
//                             MyCallbackFunction, {0.5f, 0.5f, 0.0f, 1.0f}, "LABEL");
//
// DRAW ORDER NOTE:
// ----------------------------------------------------------------------------
//   Buttons draw themselves inside Update() via GameObject::Update() -> Draw().
//   StarBackground must therefore draw in Update() BEFORE button updates,
//   not in Draw() which runs after Update() and would overdraw buttons.
// ============================================================================

#include "pch.hpp"
#include "StarBackground.hpp"
#include "MenuPage.hpp"
#include "Util.hpp"
#include "AEEngine.h"
#include <vector>
#include <iostream>
#include "Button.hpp"
#include "Input.hpp"
#include "Audio.hpp"
#include "SplashPage.hpp"

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
// Global object list (buttons self-register here via GameObject ctor)
// ============================================================================
std::vector<GameObject*> mainPageObj;

// ============================================================================
// Resources
// ============================================================================
s8    fontPath{};   // Font handle
Mouse worldMouse;   // Mouse position in world space

// ============================================================================
// Buttons (heap-allocated, deleted in Main_Free via mainPageObj loop)
// ============================================================================
Button* startButton{ nullptr };
Button* controlsButton{ nullptr };
Button* creditsButton{ nullptr };
Button* quitButton{ nullptr };
Button* backButton{ nullptr };

// ============================================================================
// Text renderers
// ============================================================================
TextRenderer mainText;     // Title text on main menu
TextRenderer controlsText; // "CONTROLS" header
TextRenderer creditsText;  // "CREDITS" header
TextRenderer infoText;     // Body text for info pages

// ============================================================================
// Art assets
// ============================================================================
SpriteRenderer creditsImg;
Transform      creditTransform;
Audio* g_MenuMusic = nullptr;

// ============================================================================
// Sub-view navigation callbacks
// ============================================================================
static void GoToControls() { currentMenuState = MENU_CONTROLS; }
static void GoToCredits() { currentMenuState = MENU_CREDITS; }
static void GoToMain() { currentMenuState = MENU_MAIN; }
static void GoToLevelSelect() { StateManagerChangeState(STATE_LEVEL_SELECT); }

// ============================================================================
// Main_Load
// ============================================================================
void Main_Load()
{
    g_MenuMusic = new Audio("Assets/Audio/MainMenusfx.wav", -1, 0.5f, 1.0f, AudioType::MUSIC);
    fontPath = AEGfxCreateFont("Assets/buggy-font.ttf", 30);
    Meshes::CreateSquareCenterOriginMesh();
    Meshes::CreateCircleMesh(); // Required by StarBackground
    StarBackground::Init();
}

// ============================================================================
// Main_Init
// ============================================================================
void Main_Init()
{
    AEGfxSetCamPosition(0.0f, 0.0f);
    currentMenuState = MENU_MAIN;
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.02f);

    mainText = TextRenderer(fontPath, 1.0f, { 0.0f, 250.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
    mainText << "HUIN!!!!!!!!";

    startButton = new Button(fontPath, { 0.0f,  100.0f }, { 300.0f, 75.0f }, GoToLevelSelect, { 0.0f, 0.6f, 0.0f, 1.0f }, "START");
    controlsButton = new Button(fontPath, { 0.0f,    0.0f }, { 300.0f, 75.0f }, GoToControls, { 0.0f, 0.3f, 0.7f, 1.0f }, "CONTROLS");
    creditsButton = new Button(fontPath, { 0.0f, -100.0f }, { 300.0f, 75.0f }, GoToCredits, { 0.5f, 0.0f, 0.5f, 1.0f }, "CREDITS");
    quitButton = new Button(fontPath, { 0.0f, -200.0f }, { 300.0f, 75.0f }, StateManagerQuit, { 0.7f, 0.0f, 0.0f, 1.0f }, "QUIT");
    backButton = new Button(fontPath, { 0.0f, -350.0f }, { 200.0f, 75.0f }, GoToMain, { 0.7f, 0.0f, 0.0f, 1.0f }, "BACK");

    controlsText = TextRenderer(fontPath, 1.0f, { 0.0f, 400.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
    controlsText << "CONTROLS";

    creditsText = TextRenderer(fontPath, 1.0f, { 0.0f, 400.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
    creditsText << "CREDITS";

    InitSpriteRenderer(creditsImg, "Assets/Credits_NTOV.png", 1600.0f, 900.0f, MESH_SQUARE);
    creditTransform = { {0.0f, 0.0f}, {1600.0f, 900.0f}, {0.0f} };

    if (g_MenuMusic) g_MenuMusic->Play();
}

// ============================================================================
// Main_Update
// ============================================================================
// Stars and background draw here (before buttons) so they appear behind them.
// Button visibility is toggled based on the active sub-view.
// ============================================================================
void Main_Update()
{
    const f32 dt = (f32)AEFrameRateControllerGetFrameTime();
    StarBackground::Update(dt);
    StarBackground::DrawBackground();
    StarBackground::Draw();

    // Credits image draws here so it sits behind the back button
    if (currentMenuState == MENU_CREDITS)
    {
        creditsImg.colour.a = 0.4f; // Semi-transparent so stars bleed through
        DrawSpriteRenderer(creditsImg, creditTransform);
    }

    const bool isMain = (currentMenuState == MENU_MAIN);
    startButton->isActive = isMain;
    controlsButton->isActive = isMain;
    creditsButton->isActive = isMain;
    quitButton->isActive = isMain;
    backButton->isActive = !isMain; // Back only shown in sub-views

    for (auto& obj : mainPageObj)
        if (obj) obj->Update(dt);
}

// ============================================================================
// Main_Draw
// ============================================================================
// Stars and buttons already drew in Update. Only text is drawn here.
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

        infoText = TextRenderer(fontPath, 1.0f, { 0.0f, 300.0f }, { 1.0f, 1.0f, 1.0f });
        infoText << "W, A, S, D  -  Move";
        infoText.Draw();

        infoText = TextRenderer(fontPath, 1.0f, { 0.0f, 200.0f }, { 1.0f, 1.0f, 1.0f });
        infoText << "Left Click  -  Shoot";
        infoText.Draw();

        infoText = TextRenderer(fontPath, 1.0f, { 0.0f, 100.0f }, { 1.0f, 1.0f, 1.0f });
        infoText << "R  -  Reload";
        infoText.Draw();

        infoText = TextRenderer(fontPath, 1.0f, { 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f });
        infoText << "E  -  Activate Invincibility";
        infoText.Draw();

        infoText = TextRenderer(fontPath, 1.0f, { 0.0f, -100.0f }, { 1.0f, 1.0f, 1.0f });
        infoText << "ESC  -  Pause";
        infoText.Draw();

        infoText = TextRenderer(fontPath, 1.0f, { 0.0f, -200.0f }, { 1.0f, 1.0f, 1.0f });
        infoText << "TAB  -  Restart";
        infoText.Draw();

        infoText = TextRenderer(fontPath, 1.0f, { 0.0f, -300.0f }, { 1.0f, 1.0f, 1.0f });
        infoText << "Q  -  Menu";
        infoText.Draw();

        // Sync all three parts of the back button so text/collider/visual align
        backButton->transform.SetPosition(0.0f, -420.0f);
        backButton->collider.position = { 0.0f, -420.0f };
        backButton->textRenderer.SetPosition({ 0.0f, -420.0f });
    }
    else if (currentMenuState == MENU_CREDITS)
    {
        // Reposition back button to bottom-left for the credits layout
        backButton->transform.SetPosition(-600.0f, -400.0f);
        backButton->collider.position = { -600.0f, -400.0f };
        backButton->textRenderer.SetPosition({ -600.0f, -400.0f });

        creditsText.Draw();
        backButton->Draw();
        backButton->textRenderer.Draw();
    }
}

// ============================================================================
// Main_Free
// ============================================================================
// Deletes all buttons via the mainPageObj list and frees the credits texture.
// ============================================================================
void Main_Free()
{
    if (g_MenuMusic) g_MenuMusic->Stop();

    for (auto* obj : mainPageObj)
        delete obj;
    mainPageObj.clear();
    mainPageObj.shrink_to_fit();

    startButton = controlsButton = creditsButton = quitButton = backButton = nullptr;

    FreeSpriteRenderer(creditsImg);
}

// ============================================================================
// Main_Unload
// ============================================================================
void Main_Unload()
{
    AEGfxDestroyFont(fontPath);
    Meshes::FreeMeshes();
    if (g_MenuMusic) { g_MenuMusic->Free();    delete g_MenuMusic;    g_MenuMusic = nullptr; }
}