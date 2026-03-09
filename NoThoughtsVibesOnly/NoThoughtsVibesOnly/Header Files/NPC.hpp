#pragma once
#include "AEEngine.h"
#include "GameObject.hpp"
#include "NPCType.hpp"
#include "Player.hpp"
#include "Particles.hpp"

// Shared NPC texture cache - loaded once in Game_Load, freed in Game_Unload
void NPC_LoadTextures();
void NPC_UnloadTextures();

class NPC : public GameObject
{
public:
	NPC();
	~NPC();
	void Start();
	void Update(f32 deltaTime);
	GameObject* target{ nullptr };
	NPCType type{ NPC_WALK };
	f32 health{ 100.0f };
	Colour baseColour{};
	bool isVisibleToPlayer = true;
	f32 fireCooldown{ 0.01f };
	f32 fireRate{ 1.0f };
	ParticleSystem explosionParticles; // Burst on death
	bool hasExploded = false;           // Ensures burst fires only once
	AEGfxTexture* NPCSpritesheet{ nullptr };
	Animation Animations;
private:
	AEVec2 velocity{ 0.0f, 0.0f };
	f32 speed{ 200.0f };
	f32 changeDirTimer = 0.0f;
	void BomberNPCs(f32 deltaTime);
	void RangerNPCs(f32 deltaTime);
	void WalkNPCs(f32 deltaTime);
	void BossNPCs(f32 deltaTime);
};