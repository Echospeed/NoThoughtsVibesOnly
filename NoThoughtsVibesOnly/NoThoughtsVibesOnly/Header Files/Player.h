#pragma once
#include "AEEngine.h"
#include "GameObject.h"

class Player : public GameObject
{
public:
	void Start();
	void Update(f32 deltaTime);

	// Area of Effect Constants
	const float AOE_RADIUS = 90.0f; // Radius of the damage circle
	const float AOE_DAMAGE = 50.0f; // Damage per second
};