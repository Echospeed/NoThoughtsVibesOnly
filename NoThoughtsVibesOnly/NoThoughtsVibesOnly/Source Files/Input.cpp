#include "pch.h"
#include "Input.h"
#include "StateManager.h"

void GetMouseWorldPosition(f32& outWorldX, f32& outWorldY)
{
	s32 mouseX, mouseY;
	AEInputGetCursorPosition(&mouseX, &mouseY);

	f32 offsetX, offsetY;
	AEGfxGetCamPosition(&offsetX, &offsetY);

	outWorldX = (static_cast<f32>(mouseX) - SCREEN_W / 2.0f) + offsetX;
	outWorldY = - (static_cast<f32>(mouseY) - SCREEN_H / 2.0f) + offsetY;
}

void GetMouseScreenPosition(f32& outScreenX, f32& outScreenY)
{
	s32 mouseX, mouseY;
	AEInputGetCursorPosition(&mouseX, &mouseY);

	outScreenX = static_cast<f32>(mouseX);
	outScreenY = static_cast<f32>(mouseY);
}