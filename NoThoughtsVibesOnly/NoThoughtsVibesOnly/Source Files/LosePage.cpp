#include "LosePage.h"
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

TextRenderer GameOverText;
Button RestartButton;

void LosePage_Load()
{
	// Load resources for the lose page
	fontPath = AEGfxCreateFont("Assets/buggy-font.ttf", 30);
	LoadTextRenderer(GameOverText, fontPath);
	LoadButton(RestartButton, fontPath);

	Meshes::CreateSquareCenterOriginMesh();
}

void LosePage_Init()
{
	// Initialize the lose page
	AEGfxSetBackgroundColor(0.1f, 0.1f, 0.15f);
	InitTextRenderer(GameOverText, "Game Over!", 1.0f, 1.0f, 0.0f, 0.0f);
	InitButton(RestartButton, "RESTART", nullptr, { 0.0f, -200.0f }, { 300.0f, 75.0f }, StateManagerMenuPage, 0.0f, 0.6f, 0.0f);
	timer = 0.0f;
}

void LosePage_Update()
{
	// Update the lose page
	GetMouseWorldPosition(worldMouse.position.x, worldMouse.position.y);

	dt = (float)AEFrameRateControllerGetFrameTime();
	timer += (float)AEFrameRateControllerGetFrameTime();
	if (timer < 6)
	{
		InitialScale += (FinalScale - InitialScale) * 0.6f * dt;
	}

	RestartButton.isHovered = isOverlapping(RestartButton.collider, worldMouse);

	if (RestartButton.isHovered)
	{
		// Change the colour of the startButton when hovered
		if (AEInputCheckTriggered(AEVK_LBUTTON))
		{
			RestartButton.onClick();
		}
	}
}

void LosePage_Draw()
{
	// Draw the lose page
	DrawTextRenderer(GameOverText, { 0.0f, 250.0f }, InitialScale);
	DrawButton(RestartButton);

}

void LosePage_Free()
{
	// Free resources for the lose page
}

void LosePage_Unload()
{
	FreeTextRenderer(GameOverText);
	FreeButton(RestartButton);
	// Unload resources for the lose page
}