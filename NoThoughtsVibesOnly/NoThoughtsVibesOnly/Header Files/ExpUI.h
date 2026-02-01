#pragma once
#include "pch.h"

struct Experience
{
	f32 CurrentExp;
	f32 MaxExp;
	f32 ExpPercentage;
	u32 CurrentLevel;
	u32 const MaxLevel;

	Experience() : CurrentExp(0.0f), MaxExp(100.0f), CurrentLevel(1), MaxLevel(100) {
		// This always runs after the initializer list is finished
		ExpPercentage = CurrentExp / MaxExp;
	}
};

void DrawExpBar(const Experience& exp, f32 xpos, f32 ypos, f32 width, f32 height);

void AddExp(Experience& exp, f32 amount);