#pragma once
#include "AEEngine.h"

struct Transform
{
	AEVec2 position{};
	AEVec2 scale{};
	f32 rotation{};
};

void TransformMovement(Transform& t);