#include "pch.h"
#include "Transform.h"

void TransformMovement(Transform& t)
{
    AEMtx33 scale, rot, trans, finalMtx;

    AEMtx33Scale(&scale, t.scale.x, t.scale.y);
    AEMtx33RotDeg(&rot, t.rotation);
    AEMtx33Trans(&trans, t.position.x, t.position.y);

    AEMtx33Concat(&finalMtx, &rot, &scale);
    AEMtx33Concat(&finalMtx, &trans, &finalMtx);
    AEGfxSetTransform(finalMtx.m);
}