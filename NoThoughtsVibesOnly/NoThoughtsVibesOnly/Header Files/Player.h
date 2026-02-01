#pragma once
#include "AEEngine.h"
#include "GameObject.h"

class Player : public GameObject
{
public:
	void Start();
	void Update(f32 deltaTime);
};