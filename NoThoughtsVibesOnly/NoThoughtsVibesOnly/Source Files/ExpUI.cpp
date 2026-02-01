#include "pch.h"
#include "ExpUI.h"
#include "Util.h"

void DrawExpBar(const Experience& exp, f32 xpos, f32 ypos, f32 width, f32 height)
{
	f32 expRatio = exp.CurrentExp / exp.MaxExp;
	if (exp.CurrentLevel >= exp.MaxLevel)
	{
		expRatio = 1.0f; // Full bar if max level reached
	}
	// Draw background bar (grey)
	CreateSquare(Meshes::pSquareLOriMesh, nullptr, nullptr, nullptr, nullptr,
		xpos, ypos, width, height, 0.0f,
		0.3f, 0.3f, 0.3f, 1.0f);
	// Draw filled portion of the bar (green)
	CreateSquare(Meshes::pSquareLOriMesh, nullptr, nullptr, nullptr, nullptr,
		xpos, ypos, width * expRatio, height, 0.0f,
		0.0f, 1.0f, 0.0f, 1.0f);
}

void AddExp(Experience& exp, f32 amount)
{
	exp.CurrentExp += amount;
	while (exp.CurrentExp >= exp.MaxExp && exp.CurrentLevel < exp.MaxLevel)
	{
		exp.CurrentExp -= exp.MaxExp;
		exp.CurrentLevel++;
		// Optionally increase MaxExp for next level
		exp.MaxExp *= 1.2f; // Example: Increase required EXP by 20% each level
	}
	if (exp.CurrentLevel >= exp.MaxLevel)
	{
		exp.CurrentExp = exp.MaxExp; // Cap EXP at max if max level reached
	}
}