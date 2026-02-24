#pragma once
#include "AEEngine.h"

struct Mouse
{
	AEVec2 position;
};

void GetMouseWorldPosition(f32& outWorldX, f32& outWorldY);
void GetMouseScreenPosition(f32& outScreenX, f32& outScreenY);