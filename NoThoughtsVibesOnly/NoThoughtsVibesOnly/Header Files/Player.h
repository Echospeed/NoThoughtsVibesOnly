#pragma once
#include "GameObject.h"

class Player : public GameObject
{
public:
    void Start();
    void Update(f32 deltaTime);
    void Shoot(AEVec2 dir);
    f32 health = 100.0f;    // starting health
    f32 maxHealth = 100.0f;

private:
    f32 shootCooldown = 0.0f;
	f32 rotationSpeed = 3.0f; // radians per second
 
};