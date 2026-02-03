#include "pch.h"
#include "ExpUI.h"
#include "Util.h"

void Experience::Load()
{
}

void Experience::Init()
{
	AEMtx33Identity(&transform);
	AEMtx33Identity(&scale);
	AEMtx33Identity(&rotation);
	AEMtx33Identity(&translation);

	CurrentExp = 0.0f;
	MaxExp = 100.0f;
	CurrentLevel = 1;

	// Default position (will be overridden by UpdatePositionWithCamera)
	EmptyBarPos = { -600.0f, 300.0f };
	BarOffset = EmptyBarPos;

	EmptyBarRot = 0.0f;
	BackgroundBarRot = 0.0f;

	// Grey background = full bar size
	BackgroundBarScale = { 300.0f, 25.0f };
	BackgroundBarPos = EmptyBarPos;

	// "EmptyBar" will be the filling portion drawn on top (starts at 0 width)
	EmptyBarScale = { 0.0f, BackgroundBarScale.y };
}

void Experience::Update(f32 deltaTime)
{
	(void)deltaTime;

	const f32 ratio = (MaxExp > 0.0f) ? (CurrentExp / MaxExp) : 0.0f;
	f32 clamped = ratio;
	if (clamped < 0.0f) clamped = 0.0f;
	if (clamped > 1.0f) clamped = 1.0f;

	// Empty bar is the portion that fills (on top of grey)
	EmptyBarScale.x = BackgroundBarScale.x * clamped;
	EmptyBarScale.y = BackgroundBarScale.y;
}

void Experience::UpdatePosition(f32 xpos, f32 ypos)
{
	EmptyBarPos = { xpos, ypos };
	BackgroundBarPos = EmptyBarPos;
}

void Experience::UpdatePositionWithCamera(f32 camX, f32 camY)
{
	UpdatePosition(BarOffset.x + camX, BarOffset.y + camY);
}

void Experience::Draw(AEGfxVertexList* Mesh)
{
	// 1) Background bar (grey) - full width
	AEGfxSetColorToMultiply(0.3f, 0.3f, 0.3f, 0.7f);

	AEMtx33Scale(&scale, BackgroundBarScale.x, BackgroundBarScale.y);
	AEMtx33Rot(&rotation, BackgroundBarRot);
	AEMtx33Trans(&translation, BackgroundBarPos.x, BackgroundBarPos.y);

	AEMtx33Concat(&transform, &rotation, &scale);
	AEMtx33Concat(&transform, &translation, &transform);

	AEGfxSetTransform(transform.m);
	AEGfxMeshDraw(Mesh, AE_GFX_MDM_TRIANGLES);

	// 2) Empty bar (the part that fills) - draw on top
	AEGfxSetColorToMultiply(0.3f, 1.0f, 0.3f, 0.7f);

	AEMtx33Scale(&scale, EmptyBarScale.x, EmptyBarScale.y);
	AEMtx33Rot(&rotation, EmptyBarRot);
	AEMtx33Trans(&translation, EmptyBarPos.x, EmptyBarPos.y);

	AEMtx33Concat(&transform, &rotation, &scale);
	AEMtx33Concat(&transform, &translation, &transform);

	AEGfxSetTransform(transform.m);
	AEGfxMeshDraw(Mesh, AE_GFX_MDM_TRIANGLES);
}

void Experience::Free()
{
}

void Experience::Unload()
{
}

void Experience::AddExp(f32 amount)
{
	CurrentExp += amount;

	// Optional clamp so it doesn't overflow visually
	if (CurrentExp > MaxExp) CurrentExp = MaxExp;
	if (CurrentExp < 0.0f) CurrentExp = 0.0f;
}