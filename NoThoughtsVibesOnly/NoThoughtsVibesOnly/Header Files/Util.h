#pragma once
#include "pch.h"

struct Square {
	f32 xpos;
	f32 ypos;
	f32 scaleX;
	f32 scaleY;
	f32 rot;
	f32 r, g, b, a;
	//Square(f32 x, f32 y, f32 sX, f32 sY, f32 rotation, f32 red, f32 green, f32 blue, f32 alpha) :
	//Square()
};

struct Circle {
	f32 xpos;
	f32 ypos;
	f32 radius;
	f32 rot;
	f32 r, g, b, a;
};

void CreateSquare(AEGfxVertexList* Mesh, AEMtx33* transform, AEMtx33* scale, AEMtx33* rotate, AEMtx33* translate,
	f32 xpos, f32 ypos, f32 scaleX, f32 scaleY, f32 rot,
	f32 r, f32 g, f32 b, f32 a);

void CreateCircle(AEGfxVertexList* Mesh, AEMtx33* transform, AEMtx33* scale, AEMtx33* rotate, AEMtx33* translate,
	f32 xpos, f32 ypos, f32 radius, f32 rot,
	f32 r, f32 g, f32 b, f32 a);