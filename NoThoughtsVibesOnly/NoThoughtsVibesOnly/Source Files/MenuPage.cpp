#include "pch.hpp"
#include "MenuPage.hpp"
#include "Util.hpp"
#include "AEEngine.h"
#include <vector>
#include <iostream>
#include "Button.hpp"
#include "Input.hpp"

// ---------------------------------------------------------------------------
// Struct GameObject and Variables
// ---------------------------------------------------------------------------
enum MenuState {
	MENU_MAIN,
	MENU_CONTROLS,
	MENU_CREDITS
};

static MenuState currentMenuState = MENU_MAIN;

// ---------------------------------------------------------------------------
// Global/Static Object Vectors
// ---------------------------------------------------------------------------
std::vector<GameObject*> mainPageObj;

// Font 
s8 fontPath{};

// Mouse
Mouse worldMouse;

// Buttons
Button* startButton{ nullptr };
Button* controlsButton{ nullptr };
Button* creditsButton{ nullptr };
Button* quitButton{ nullptr };
Button* backButton{ nullptr };

// Text Renderers
TextRenderer mainText;
TextRenderer controlsText; // Header for controls
TextRenderer creditsText;  // Header for credits
TextRenderer infoText;     // Body text for details


// ---------------------------------------------------------------------------
// Helper Functions for Button Transitions
// ---------------------------------------------------------------------------
void GoToControls() { currentMenuState = MENU_CONTROLS; }
void GoToCredits() { currentMenuState = MENU_CREDITS; }
void GoToMain() { currentMenuState = MENU_MAIN; }

// ---------------------------------------------------------------------------
// Main Functions
// ---------------------------------------------------------------------------

void Main_Load()
{
	fontPath = AEGfxCreateFont("Assets/buggy-font.ttf", 30);

	// Load Main Buttons
	// Load Back Button

	// Load Text
	LoadTextRenderer(mainText, fontPath);
	LoadTextRenderer(controlsText, fontPath);
	LoadTextRenderer(creditsText, fontPath);
	LoadTextRenderer(infoText, fontPath);

	Meshes::CreateSquareCenterOriginMesh();
}

void Main_Init()
{
	// Reset the camera
	AEGfxSetCamPosition(0.0f, 0.0f);
	currentMenuState = MENU_MAIN;

	// Initialize Main Menu UI
	mainText.SetText("HUIN!!!!!!!!");

	// Link Start button to game state
	startButton = new Button(fontPath, { 0.0f, 100.0f }, { 300.0f, 75.0f }, StateManagerGamePage, { 0.0f, 0.6f, 0.0f, 1.0f });
	startButton->textRenderer.SetText("START");

	// Control Button
	controlsButton = new Button(fontPath, { 0.0f, 0.0f }, { 300.0f, 75.0f }, GoToControls, { 0.0f, 0.3f, 0.7f, 1.0f });
	controlsButton->textRenderer.SetText("CONTROLS");

	// Credit Button
	creditsButton = new Button(fontPath, { 0.0f, -100.0f }, { 300.0f, 75.0f }, GoToCredits, { 0.5f, 0.0f, 0.5f, 1.0f });
	creditsButton->textRenderer.SetText("CREDITS");

	// Quit Button
	quitButton = new Button(fontPath, { 0.0f, -200.0f }, { 300.0f, 75.0f }, StateManagerQuit, { 0.7f, 0.0f, 0.0f, 1.0f });
	quitButton->textRenderer.SetText("QUIT");

	// Back Bautton
	backButton = new Button(fontPath, { 0.0f, -350.0f }, { 200.0f, 75.0f }, GoToMain, { 0.7f, 0.0f, 0.0f, 1.0f });
	backButton->textRenderer.SetText("BACK"); //=> Set Draw Function in Class to draw.

	// Initialize Headers
	InitTextRenderer(controlsText, "CONTROLS", { 1.0f, 1.0f }, 1.0f, 1.0f, 1.0f);
	InitTextRenderer(creditsText, "CREDITS", { 1.0f, 1.0f }, 1.0f, 1.0f, 1.0f);

	AEGfxSetBackgroundColor(0.1f, 0.1f, 0.15f);
}

void Main_Update()
{
	f32 dt = (f32)AEFrameRateControllerGetFrameTime();

	for (auto& obj : mainPageObj)
	{
		if (!obj) continue;
		obj->Update(dt);
		
	}
}

void Main_Draw()
{
	if (currentMenuState == MENU_MAIN)
	{
		// Draw Main Menu
		DrawTextRenderer(mainText, { 0.0f, 250.0f }, 1.5f);
		/*DrawButton(startButton);
		DrawButton(controlsButton);
		DrawButton(creditsButton);
		DrawButton(quitButton);*/
	}
	else if (currentMenuState == MENU_CONTROLS)
	{
		// Draw Controls Page
		DrawTextRenderer(controlsText, { 0.0f, 250.0f }, 1.5f);

		// Text Content for Controls
		InitTextRenderer(infoText, "W, A, S, D - Move Character", { 1.0f, 1.0f }, 1.0f, 1.0f, 1.0f);
		DrawTextRenderer(infoText, { 0.0f, 100.0f }, 1.0f);

		InitTextRenderer(infoText, "Q - Return to Main Menu", { 1.0f, 1.0f }, 1.0f, 1.0f, 1.0f);
		DrawTextRenderer(infoText, { 0.0f, 0.0f }, 1.0f);

		InitTextRenderer(infoText, "R - Restart Level", { 1.0f, 1.0f }, 1.0f, 1.0f, 1.0f);
		DrawTextRenderer(infoText, { 0.0f, -100.0f }, 1.0f);

		//DrawButton(backButton);
	}
	else if (currentMenuState == MENU_CREDITS)
	{
		// Draw Credits Page
		DrawTextRenderer(creditsText, { 0.0f, 250.0f }, 1.5f);

		// Text Content for Credits
		InitTextRenderer(infoText, "Created By:", { 1.0f, 1.0f }, 1.0f, 1.0f, 1.0f);
		DrawTextRenderer(infoText, { 0.0f, 100.0f }, 1.0f);

		InitTextRenderer(infoText, "No Thoughts, Vibes Only", { 1.0f, 1.0f }, 1.0f, 1.0f, 1.0f);
		DrawTextRenderer(infoText, { 0.0f, 0.0f }, 1.0f);

		InitTextRenderer(infoText, "DigiPen Institute of Technology", { 1.0f, 1.0f }, 0.7f, 0.7f, 1.0f); // Cyan
		DrawTextRenderer(infoText, { 0.0f, -100.0f }, 0.8f);

		//DrawButton(backButton);
	}
}

void Main_Free()
{
	FreeTextRenderer(mainText);
	FreeTextRenderer(controlsText);
	FreeTextRenderer(creditsText);
	FreeTextRenderer(infoText);

	delete startButton;     startButton = nullptr;
	delete controlsButton;  controlsButton = nullptr;
	delete creditsButton;   creditsButton = nullptr;
	delete quitButton;      quitButton = nullptr;
	delete backButton;      backButton = nullptr;

	//for (auto& obj : mainPageObj) { delete obj; obj = nullptr; }

	//// 3. Clear the vector and reset global pointers
	mainPageObj.clear();
	mainPageObj.shrink_to_fit();
}

void Main_Unload()
{
	AEGfxDestroyFont(fontPath);
	Meshes::FreeMeshes();
}