#include "Util.h"

namespace Meshes{
	AEGfxVertexList* pSquareCOriMesh = nullptr;
	AEGfxVertexList* pSquareLOriMesh = nullptr;
	AEGfxVertexList* pCircleMesh = nullptr;

	// Standard Square Mesh with center origin for center scaling/rotation transformations
	void CreateSquareCenterOriginMesh()
	{
		AEGfxMeshStart();

		AEGfxTriAdd(
			-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
			0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
			-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f
		);
		AEGfxTriAdd(
			0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
			0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
			-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f
		);

		pSquareCOriMesh = AEGfxMeshEnd();
	}

	// Standard Square Mesh with left origin for left-aligned scaling/rotation transformations
	void CreateSquareLeftOriginMesh()
	{
		AEGfxMeshStart();

		AEGfxTriAdd(
			0.0f, 0.0f, 0xFFFFFFFF, 0.0f, 1.0f,
			1.0f, 0.0f, 0xFFFFFFFF, 1.0f, 1.0f,
			0.0f, 1.0f, 0xFFFFFFFF, 0.0f, 0.0f
		);
		AEGfxTriAdd(
			1.0f, 0.0f, 0xFFFFFFFF, 1.0f, 1.0f,
			1.0f, 1.0f, 0xFFFFFFFF, 1.0f, 0.0f,
			0.0f, 1.0f, 0xFFFFFFFF, 0.0f, 0.0f
		);
		pSquareLOriMesh = AEGfxMeshEnd();
	}

	// Standard Circle Mesh for UI
	void CreateCircleMesh()
	{
		AEGfxMeshStart();

		int steps = 32;
		float angleStep = (2.0f * 3.14159f) / (float)steps;

		for (int i = 0; i < steps; ++i)
		{
			float theta1 = i * angleStep;
			float theta2 = (i + 1) * angleStep;

			// Center (0,0), Point A, Point B - White Color (0xFFFFFFFF)
			AEGfxTriAdd(
				0.0f, 0.0f, 0xFFFFFFFF, 0.0f, 0.0f,
				cosf(theta1), sinf(theta1), 0xFFFFFFFF, 0.0f, 0.0f,
				cosf(theta2), sinf(theta2), 0xFFFFFFFF, 0.0f, 0.0f
			);
		}

		pCircleMesh = AEGfxMeshEnd();
	}

	void FreeMeshes()
	{
		if (pSquareCOriMesh){
			AEGfxMeshFree(pSquareCOriMesh);
			pSquareCOriMesh = nullptr;
		}
		if (pSquareLOriMesh)
		{
			AEGfxMeshFree(pSquareLOriMesh);
			pSquareLOriMesh = nullptr;
		}
		if (pCircleMesh)
		{
			AEGfxMeshFree(pCircleMesh);
			pCircleMesh = nullptr;
		}
	}
}

void CreateSquare(AEGfxVertexList* Mesh, AEMtx33* transform, AEMtx33* scale, AEMtx33* rotate, AEMtx33* translate,
	f32 xpos, f32 ypos, f32 scaleX, f32 scaleY, f32 rot,
	f32 r, f32 g, f32 b, f32 a) {


	AEGfxSetColorToMultiply(r, g, b, a);

	AEMtx33Scale(scale, scaleX, scaleY);
	AEMtx33Rot(rotate, rot);
	AEMtx33Trans(translate, xpos, ypos);

	AEMtx33Concat(transform, rotate, scale);
	AEMtx33Concat(transform, translate, transform);

	AEGfxSetTransform(transform->m);
	AEGfxMeshDraw(Mesh, AE_GFX_MDM_TRIANGLES);

}

void CreateCircle(AEGfxVertexList* Mesh, AEMtx33* transform, AEMtx33* scale, AEMtx33* rotate, AEMtx33* translate,
	f32 xpos, f32 ypos, f32 radius, f32 rot,
	f32 r, f32 g, f32 b, f32 a)
{

}