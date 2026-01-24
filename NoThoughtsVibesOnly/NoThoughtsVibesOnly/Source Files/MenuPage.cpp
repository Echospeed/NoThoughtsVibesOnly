#include "MenuPage.h"
#include "Util.h"
#include "AEEngine.h"
#include <vector>
#include <iostream>
#include "Button.h"
#include "Input.h"


// ---------------------------------------------------------------------------
// Stuct Objects and Variables
// ---------------------------------------------------------------------------

//Mouse
Mouse worldMouse; // World Position Mouse: Used on UI Game Object

//Buttons
Button startButton;
Button controlsButton;
Button creditsButton;
Button quitButton;

//Text Renderer
TextRenderer mainText;

//Font 
s8 fontPath{};

void Main_Load()
{
	fontPath = AEGfxCreateFont("Assets/buggy-font.ttf", 30);
	LoadButton(startButton, fontPath);
	LoadButton(controlsButton, fontPath);
	LoadButton(creditsButton, fontPath);
	LoadButton(quitButton, fontPath);
	LoadTextRenderer(mainText, fontPath);

	Meshes::CreateSquareCenterOriginMesh();
}

void Main_Init()
{
	InitTextRenderer(mainText, "No Thoughts, Vibes Only", 1.0f, 1.0f, 1.0f, 1.0f);
	InitButton(startButton, "START", nullptr, { 0.0f, 100.0f }, { 300.0f, 75.0f }, StateManagerGamePage, 0.0f, 0.6f, 0.0f);
	InitButton(controlsButton, "CONTROL", nullptr, { 0.0f, 0.0f }, { 300.0f, 75.0f }, nullptr, 0.0f, 0.3f, 0.7f);
	InitButton(creditsButton, "CREDITS", nullptr, { 0.0f, -100.0f }, { 300.0f, 75.0f }, nullptr, 0.5f, 0.0f, 0.5f);
	InitButton(quitButton, "QUIT", nullptr, { 0.0f, -200.0f }, { 300.0f, 75.0f }, StateManagerQuit, 0.7f, 0.0f, 0.0f);
	AEGfxSetBackgroundColor(0.1f, 0.1f, 0.15f);
}

void Main_Update()
{
	GetMouseWorldPosition(worldMouse.position.x, worldMouse.position.y);

	startButton.isHovered = isOverlapping(startButton.collider, worldMouse);
	quitButton.isHovered = isOverlapping(quitButton.collider, worldMouse);

	if (startButton.isHovered)
	{
		// Change the colour of the startButton when hovered
		if(AEInputCheckTriggered(AEVK_LBUTTON))
		{
			startButton.onClick();
		}
	}
	else if (quitButton.isHovered)
	{
		// Change the colour of the startButton when hovered
		if (AEInputCheckTriggered(AEVK_LBUTTON))
		{
			quitButton.onClick();
		}
	}
}

void Main_Draw()
{
	DrawTextRenderer(mainText, { 0.0f, 250.0f }, 1.5f);
	DrawButton(startButton);
	DrawButton(controlsButton);
	DrawButton(creditsButton);
	DrawButton(quitButton);
}

void Main_Free()
{
	FreeButton(startButton);
	FreeButton(controlsButton);
	FreeButton(creditsButton);
	FreeButton(quitButton);
	FreeTextRenderer(mainText);
}

void Main_Unload()
{
	AEGfxDestroyFont(fontPath);
	Meshes::FreeMeshes();
}