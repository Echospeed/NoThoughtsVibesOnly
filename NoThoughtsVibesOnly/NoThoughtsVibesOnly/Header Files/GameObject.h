#pragma once


#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H
//Defines a class structure to manage game entities using the engine's math and graphics classes.
#include "AEEngine.h"

class GameObject {
public:
    // Uses AEVec2 for 2D positioning as defined in engine classes
    AEVec2 pos;
    AEVec2 velocity;
    float  rotation;    // Rotation in radians
    float  scale;
    unsigned int color; // Color in 0xAARRGGBB format

    // Pointer to the mesh data handled by AEGfx
    AEGfxVertexList* pMesh;

    GameObject();

    // Modular methods for entity logic
    void Update(float dt);
    void Draw();
};

#endif
