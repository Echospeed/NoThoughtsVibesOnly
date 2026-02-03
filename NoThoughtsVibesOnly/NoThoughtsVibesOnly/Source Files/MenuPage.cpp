#include "pch.h"
#include "MenuPage.h"
#include "Util.h"
#include "AEEngine.h"
#include <vector>
#include <iostream>
#include "Button.h"
#include "Input.h"

// ---------------------------------------------------------------------------
// Struct Objects and Variables
// ---------------------------------------------------------------------------
enum MenuState {
	MENU_MAIN,
	MENU_CONTROLS,
	MENU_CREDITS
};

static MenuState currentMenuState = MENU_MAIN;

// Mouse
Mouse worldMouse;

// Buttons
Button startButton;
Button controlsButton;
Button creditsButton;
Button quitButton;
Button backButton;

// Text Renderers
TextRenderer mainText;
TextRenderer controlsText; // Header for controls
TextRenderer creditsText;  // Header for credits
TextRenderer infoText;     // Body text for details

// Font 
s8 fontPath{};

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
	LoadButton(startButton, fontPath);
	LoadButton(controlsButton, fontPath);
	LoadButton(creditsButton, fontPath);
	LoadButton(quitButton, fontPath);

	// Load Back Button
	LoadButton(backButton, fontPath);

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
	InitTextRenderer(mainText, "No Thoughts, Vibes Only", 1.0f, 1.0f, 1.0f, 1.0f);

	// Link Start button to game state
	InitButton(startButton, "START", nullptr, { 0.0f, 100.0f }, { 300.0f, 75.0f }, StateManagerGamePage, 0.0f, 0.6f, 0.0f);

	// Link Controls/Credits buttons to local helper functions
	InitButton(controlsButton, "CONTROLS", nullptr, { 0.0f, 0.0f }, { 300.0f, 75.0f }, GoToControls, 0.0f, 0.3f, 0.7f);
	InitButton(creditsButton, "CREDITS", nullptr, { 0.0f, -100.0f }, { 300.0f, 75.0f }, GoToCredits, 0.5f, 0.0f, 0.5f);

	// Link Quit button to quit application
	InitButton(quitButton, "QUIT", nullptr, { 0.0f, -200.0f }, { 300.0f, 75.0f }, StateManagerQuit, 0.7f, 0.0f, 0.0f);

	// Initialize Sub-Menu UI
	InitButton(backButton, "BACK", nullptr, { 0.0f, -350.0f }, { 200.0f, 75.0f }, GoToMain, 0.7f, 0.0f, 0.0f);

	// Initialize Headers
	InitTextRenderer(controlsText, "CONTROLS", 1.0f, 1.0f, 1.0f, 1.0f);
	InitTextRenderer(creditsText, "CREDITS", 1.0f, 1.0f, 1.0f, 1.0f);

	AEGfxSetBackgroundColor(0.1f, 0.1f, 0.15f);
}

void Main_Update()
{
	GetMouseWorldPosition(worldMouse.position.x, worldMouse.position.y);

	if (currentMenuState == MENU_MAIN)
	{
		// Update Main Menu Buttons
		startButton.isHovered = isOverlapping(startButton.collider, worldMouse);
		controlsButton.isHovered = isOverlapping(controlsButton.collider, worldMouse);
		creditsButton.isHovered = isOverlapping(creditsButton.collider, worldMouse);
		quitButton.isHovered = isOverlapping(quitButton.collider, worldMouse);

		if (startButton.isHovered && AEInputCheckTriggered(AEVK_LBUTTON))
			startButton.onClick();

		else if (controlsButton.isHovered && AEInputCheckTriggered(AEVK_LBUTTON))
			controlsButton.onClick();

		else if (creditsButton.isHovered && AEInputCheckTriggered(AEVK_LBUTTON))
			creditsButton.onClick();

		else if (quitButton.isHovered && AEInputCheckTriggered(AEVK_LBUTTON))
			quitButton.onClick();
	}
	else if (currentMenuState == MENU_CONTROLS || currentMenuState == MENU_CREDITS)
	{
		// Update Back Button
		backButton.isHovered = isOverlapping(backButton.collider, worldMouse);

		if (backButton.isHovered && AEInputCheckTriggered(AEVK_LBUTTON))
			backButton.onClick();
	}
}

void Main_Draw()
{
	if (currentMenuState == MENU_MAIN)
	{
		// Draw Main Menu
		DrawTextRenderer(mainText, { 0.0f, 250.0f }, 1.5f);
		DrawButton(startButton);
		DrawButton(controlsButton);
		DrawButton(creditsButton);
		DrawButton(quitButton);
	}
	else if (currentMenuState == MENU_CONTROLS)
	{
		// Draw Controls Page
		DrawTextRenderer(controlsText, { 0.0f, 250.0f }, 1.5f);

		// Text Content for Controls
		InitTextRenderer(infoText, "W, A, S, D - Move Character", 1.0f, 1.0f, 1.0f, 1.0f);
		DrawTextRenderer(infoText, { 0.0f, 100.0f }, 1.0f);

		InitTextRenderer(infoText, "Q - Return to Main Menu", 1.0f, 1.0f, 1.0f, 1.0f);
		DrawTextRenderer(infoText, { 0.0f, 0.0f }, 1.0f);

		InitTextRenderer(infoText, "R - Restart Level", 1.0f, 1.0f, 1.0f, 1.0f);
		DrawTextRenderer(infoText, { 0.0f, -100.0f }, 1.0f);

		DrawButton(backButton);
	}
	else if (currentMenuState == MENU_CREDITS)
	{
		// Draw Credits Page
		DrawTextRenderer(creditsText, { 0.0f, 250.0f }, 1.5f);

		// Text Content for Credits
		InitTextRenderer(infoText, "Created By:", 1.0f, 1.0f, 1.0f, 1.0f);
		DrawTextRenderer(infoText, { 0.0f, 100.0f }, 1.0f);

		InitTextRenderer(infoText, "No Thoughts, Vibes Only", 0.0f, 1.0f, 1.0f, 1.0f);
		DrawTextRenderer(infoText, { 0.0f, 0.0f }, 1.0f);

		InitTextRenderer(infoText, "DigiPen Institute of Technology", 0.7f, 0.7f, 0.7f, 1.0f); // Cyan
		DrawTextRenderer(infoText, { 0.0f, -100.0f }, 0.8f);

		DrawButton(backButton);
	}
}

void Main_Free()
{
	FreeButton(startButton);
	FreeButton(controlsButton);
	FreeButton(creditsButton);
	FreeButton(quitButton);
	FreeButton(backButton);

	FreeTextRenderer(mainText);
	FreeTextRenderer(controlsText);
	FreeTextRenderer(creditsText);
	FreeTextRenderer(infoText);
}

void Main_Unload()
{
	AEGfxDestroyFont(fontPath);
	Meshes::FreeMeshes();
}