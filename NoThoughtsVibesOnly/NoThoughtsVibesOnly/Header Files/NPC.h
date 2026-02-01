#pragma once
#include "AEEngine.h"
#include "GameObject.h"
#include "NPCType.h"
#include "Player.h"

class NPC : public GameObject
{
public:
	void Start();
	void Update(f32 deltaTime);
	GameObject* target;
	NPCType type{NPC_WALK};
	f32 health{ 100.0f };
	Colour baseColour{};
private:
	AEVec2 velocity{ 0.0f, 0.0f };
	f32 speed{ 200.0f };
	void BomberNPCs(f32 deltaTime);
	void RangerNPCs(f32 deltaTime);
	void WalkNPCs(f32 deltaTime);
};