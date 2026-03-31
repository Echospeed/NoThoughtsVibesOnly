#pragma once
// Author: Chia Wenjie
// Co-Author: John Chiow, Liu Yan Bin, Stanley Lu

#include <vector>
#include "GameObject.hpp"
#include "Particles.hpp"

void Game_Load();
	 
void Game_Init();
	 
void Game_Update();
	 
void Game_Draw();
	 
void Game_Free();
	 
void Game_Unload();

extern std::vector<GameObject*> gamePageObj;
extern f32 WORLD_WIDTH;
extern f32 WORLD_HEIGHT;
extern int availableBullets;
extern int g_FinalScore;
extern int g_FinalWaveCount;
extern ParticleSystem g_EnemyHitPS;
extern ParticleSystem g_PlayerHitPS;