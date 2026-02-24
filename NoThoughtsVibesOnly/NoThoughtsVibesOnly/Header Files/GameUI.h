#pragma once
#include "pch.h"

struct GameUI
{
	// ---------------------------------------------------------------------------
	// AMMO TEXT
	// ---------------------------------------------------------------------------
	TextRenderer ammoText;
	s8 gameFont = 0; // Handle for the font
	char ammoBuffer[500]; // Buffer to hold the text string "Ammo: 50"


	// UI elements 
	AEVec2 BarOffset; // Offset from camera position
	AEVec2 HealthBarOffset; // Offset for health bar

	//EXP
	AEVec2 ExpBarPos;
	AEVec2 ExpBarScale;
	f32 ExpBarRot;

	AEVec2 BGExpBarPos;
	AEVec2 BGExpBarScale;
	f32 BGExpBarRot;

	//HEALTH
	AEVec2 HealthBarPos;
	AEVec2 HealthBarScale;
	f32 HealthBarRot;

	AEVec2 BGHealthBarPos;
	AEVec2 BGHealthBarScale;
	f32 BGHealthBarRot;

	//Stats
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

	//function updates
	void UpdatePosition(f32 xpos, f32 ypos);

	// New: update bar position using camera + UI offsets
	void UpdatePositionWithCamera(f32 camX, f32 camY);

	void UpdateHealth(f32 currentHealth, f32 maxHealth);

	void AddExp(f32 amount);
};
