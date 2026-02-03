#pragma once
#include <vector>
#include "GameObject.h"

void Game_Load();
	 
void Game_Init();
	 
void Game_Update();
	 
void Game_Draw();
	 
void Game_Free();
	 
void Game_Unload();

extern std::vector<GameObject*> objects;
extern const f32 WORLD_WIDTH;
extern const f32 WORLD_HEIGHT;
extern int availableBullets;