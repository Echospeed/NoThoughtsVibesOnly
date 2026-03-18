#pragma once
#include <vector>
#include "GameObject.hpp"

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