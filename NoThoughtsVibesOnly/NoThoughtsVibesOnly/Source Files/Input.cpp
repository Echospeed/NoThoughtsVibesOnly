#include "Input.h"
#include "StateManager.h"

void GetMouseWorldPosition(f32& outWorldX, f32& outWorldY)
{
	s32 mouseX, mouseY;
	AEInputGetCursorPosition(&mouseX, &mouseY);
	outWorldX = static_cast<f32>(mouseX) - SCREEN_W / 2.0f;
	outWorldY = - (static_cast<f32>(mouseY) - SCREEN_H / 2.0f);
}

void GetMouseScreenPosition(f32& outScreenX, f32& outScreenY)
{
	s32 mouseX, mouseY;
	AEInputGetCursorPosition(&mouseX, &mouseY);

	outScreenX = static_cast<f32>(mouseX);
	outScreenY = static_cast<f32>(mouseY);
}