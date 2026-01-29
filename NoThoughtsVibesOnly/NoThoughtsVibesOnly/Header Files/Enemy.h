#pragma once
#include "AEEngine.h"
#include "GameObject.h"

class Enemy : public GameObject
{
public:
	void Start();
	void Update(f32 deltaTime);
};