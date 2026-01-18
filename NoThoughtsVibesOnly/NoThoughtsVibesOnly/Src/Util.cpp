#include "Util.h"

//Square()
//Square()

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