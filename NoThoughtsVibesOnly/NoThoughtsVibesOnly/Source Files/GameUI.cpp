#include "pch.h"
#include "GameUI.h"
#include "Util.h"

void GameUI::Load()
{
	gameFont = AEGfxCreateFont("Assets/buggy-font.ttf", 30);
	LoadTextRenderer(ammoText, gameFont);
}

void GameUI::Init()
{
	// Initialize Ammo UI (Top Left corner usually)
	// Position x=-700, y=400 puts it in top-left area relative to camera center if UI follows camera
	// But since we draw UI *after* resetting camera, we use screen coordinates logic.
	InitTextRenderer(ammoText, "Ammo: 100%", 1.0f, 1.0f, 1.0f, 1.0f); // White text

	AEMtx33Identity(&transform);
	AEMtx33Identity(&scale);
	AEMtx33Identity(&rotation);
	AEMtx33Identity(&translation);

	CurrentExp = 0.0f;
	MaxExp = 100.0f;
	CurrentLevel = 1;

	// EXP bar offset from camera
	BarOffset = { -600.0f, 250.0f };
	ExpBarPos = BarOffset;
	ExpBarRot = 0.0f;
	BGExpBarRot = 0.0f;

	// Grey background = full bar size
	BGExpBarScale = { 300.0f, 25.0f };
	BGExpBarPos = ExpBarPos;

	// EXP fill bar (starts at 0 width)
	ExpBarScale = { 0.0f, BGExpBarScale.y };

	// Health bar offset (below EXP bar)
	HealthBarOffset = { -600.0f, 300.0f };
	HealthBarPos = HealthBarOffset;
	HealthBarRot = 0.0f;
	BGHealthBarRot = 0.0f;

	// Health background (full width)
	BGHealthBarScale = { 300.0f, 25.0f };
	BGHealthBarPos = HealthBarPos;

	// Health fill bar (starts full)
	HealthBarScale = { BGHealthBarScale.x, BGHealthBarScale.y };
}

void GameUI::Update(f32 deltaTime)
{
	(void)deltaTime;

	const f32 ratio = (MaxExp > 0.0f) ? (CurrentExp / MaxExp) : 0.0f;
	f32 clamped = ratio;
	if (clamped < 0.0f) clamped = 0.0f;
	if (clamped > 1.0f) clamped = 1.0f;

	// EXP bar fill
	ExpBarScale.x = BGExpBarScale.x * clamped;
	ExpBarScale.y = BGExpBarScale.y;

	sprintf_s(ammoBuffer, "Ammo: %d / %d", availableBullets, 500);
	ammoText.text = ammoBuffer;
}

void GameUI::UpdatePosition(f32 xpos, f32 ypos)
{
	ExpBarPos = { xpos, ypos };
	BGExpBarPos = ExpBarPos;

	// Health bar follows at its own offset
	HealthBarPos = { HealthBarOffset.x + (xpos - BarOffset.x), HealthBarOffset.y + (ypos - BarOffset.y) };
	BGHealthBarPos = HealthBarPos;
}

void GameUI::UpdatePositionWithCamera(f32 camX, f32 camY)
{
	UpdatePosition(BarOffset.x + camX, BarOffset.y + camY);
}

void GameUI::UpdateHealth(f32 currentHealth, f32 maxHealth)
{
	f32 ratio = (maxHealth > 0.0f) ? (currentHealth / maxHealth) : 0.0f;
	if (ratio < 0.0f) ratio = 0.0f;
	if (ratio > 1.0f) ratio = 1.0f;

	// Scale based on background width, NOT itself
	HealthBarScale.x = BGHealthBarScale.x * ratio;
	HealthBarScale.y = BGHealthBarScale.y;
}



void GameUI::Draw(AEGfxVertexList* Mesh)
{
	// Draw MiniMap
	DrawTextRenderer(ammoText, { -500.0f , 400.0f }, 1.0f);

	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	//// 1) EXP Background bar (grey)
	//AEGfxSetColorToMultiply(0.3f, 0.3f, 0.3f, 0.7f);

	//AEMtx33Scale(&scale, BGExpBarScale.x, BGExpBarScale.y);
	//AEMtx33Rot(&rotation, BGExpBarRot);
	//AEMtx33Trans(&translation, BGExpBarPos.x, BGExpBarPos.y);

	//AEMtx33Concat(&transform, &rotation, &scale);
	//AEMtx33Concat(&transform, &translation, &transform);

	//AEGfxSetTransform(transform.m);
	//AEGfxMeshDraw(Mesh, AE_GFX_MDM_TRIANGLES);

	//// 2) EXP fill bar (green)
	//AEGfxSetColorToMultiply(0.3f, 1.0f, 0.3f, 0.7f);

	//AEMtx33Scale(&scale, ExpBarScale.x, ExpBarScale.y);
	//AEMtx33Rot(&rotation, ExpBarRot);
	//AEMtx33Trans(&translation, ExpBarPos.x, ExpBarPos.y);

	//AEMtx33Concat(&transform, &rotation, &scale);
	//AEMtx33Concat(&transform, &translation, &transform);

	//AEGfxSetTransform(transform.m);
	//AEGfxMeshDraw(Mesh, AE_GFX_MDM_TRIANGLES);

	// 3) Health background bar (dark red)
	AEGfxSetColorToMultiply(0.3f, 0.0f, 0.0f, 0.7f);

	AEMtx33Scale(&scale, BGHealthBarScale.x, BGHealthBarScale.y);
	AEMtx33Rot(&rotation, BGHealthBarRot);
	AEMtx33Trans(&translation, BGHealthBarPos.x, BGHealthBarPos.y);

	AEMtx33Concat(&transform, &rotation, &scale);
	AEMtx33Concat(&transform, &translation, &transform);

	AEGfxSetTransform(transform.m);
	AEGfxMeshDraw(Mesh, AE_GFX_MDM_TRIANGLES);

	// 4) Health fill bar (red)
	AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 0.7f);

	AEMtx33Scale(&scale, HealthBarScale.x, HealthBarScale.y);
	AEMtx33Rot(&rotation, HealthBarRot);
	AEMtx33Trans(&translation, HealthBarPos.x, HealthBarPos.y);

	AEMtx33Concat(&transform, &rotation, &scale);
	AEMtx33Concat(&transform, &translation, &transform);

	AEGfxSetTransform(transform.m);
	AEGfxMeshDraw(Mesh, AE_GFX_MDM_TRIANGLES);
}

void GameUI::Free()
{
}

void GameUI::Unload()
{
	AEGfxDestroyFont(gameFont);
}

void GameUI::AddExp(f32 amount)
{
	CurrentExp += amount;

	if (CurrentExp > MaxExp) CurrentExp = MaxExp;
	if (CurrentExp < 0.0f) CurrentExp = 0.0f;
}