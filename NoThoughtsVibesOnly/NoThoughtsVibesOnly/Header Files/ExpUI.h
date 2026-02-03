#pragma once
#include "pch.h"

struct Experience
{
	AEVec2 BarOffset; // Offset from camera position

	AEVec2 EmptyBarPos;
	AEVec2 EmptyBarScale;
	f32 EmptyBarRot;

	AEVec2 BackgroundBarPos;
	AEVec2 BackgroundBarScale;
	f32 BackgroundBarRot;

	f32 CurrentExp;
	f32 MaxExp;
	u32 CurrentLevel;
	u32 const MaxLevel;

	AEMtx33 transform, scale, rotation, translation;

	void Load();
	void Init();
	void Update(f32 deltaTime);
	void Draw(AEGfxVertexList*);
	void Free();
	void Unload();

	void UpdatePosition(f32 xpos, f32 ypos);

	// New: update bar position using camera + UI offsets
	void UpdatePositionWithCamera(f32 camX, f32 camY);

	void AddExp(f32 amount);
};
