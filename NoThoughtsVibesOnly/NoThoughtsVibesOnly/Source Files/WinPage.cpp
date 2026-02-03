#include "pch.h"
#include "WinPage.h"
#include "AEEngine.h"
#include "Util.h"
#include "Button.h"

namespace {
	Mouse worldMouse;
	s8 fontPath{};
	f32 InitialScale = 1.5f;
	f32 FinalScale = 3.5f;
	f32 dt = 0.0f;
	f32 timer = 0.0f;
}

TextRenderer WinText;
Button WinButton;

void WinPage_Load()
{
	// Load resources for the lose page
	fontPath = AEGfxCreateFont("Assets/buggy-font.ttf", 30);
	LoadTextRenderer(WinText, fontPath);
	LoadButton(WinButton, fontPath);

	Meshes::CreateSquareCenterOriginMesh();
}

void WinPage_Init()
{
	// Initialize the lose page
	AEGfxSetBackgroundColor(0.1f, 0.1f, 0.15f);
	InitialScale = 1.5f;
	InitTextRenderer(WinText, "YOU WIN!", 1.0f, 1.0f, 0.0f, 0.0f);
	InitButton(WinButton, "RESTART", nullptr, { 0.0f, -200.0f }, { 300.0f, 75.0f }, StateManagerMenuPage, 0.0f, 0.6f, 0.0f);
	timer = 0.0f;
}

void WinPage_Update()
{
	AEGfxSetCamPosition(0.0f, 0.0f);
	// Update the lose page
	GetMouseWorldPosition(worldMouse.position.x, worldMouse.position.y);

	dt = (f32)AEFrameRateControllerGetFrameTime();
	timer += (f32)AEFrameRateControllerGetFrameTime();
	if (timer < 6)
	{
		InitialScale += (FinalScale - InitialScale) * 0.6f * dt;
	}

	WinButton.isHovered = isOverlapping(WinButton.collider, worldMouse);

	if (WinButton.isHovered)
	{
		// Change the colour of the startButton when hovered
		if (AEInputCheckTriggered(AEVK_LBUTTON))
		{
			WinButton.onClick();
		}
	}
}

void WinPage_Draw()
{
	// Draw the lose page
	DrawTextRenderer(WinText, { 0.0f, 250.0f }, InitialScale);
	DrawButton(WinButton);

}

void WinPage_Free()
{
	// Free resources for the Win page
	FreeTextRenderer(WinText);
	FreeButton(WinButton);
}

void WinPage_Unload()
{
	AEGfxDestroyFont(fontPath);
	Meshes::FreeMeshes();
	// Unload resources for the Win page
}