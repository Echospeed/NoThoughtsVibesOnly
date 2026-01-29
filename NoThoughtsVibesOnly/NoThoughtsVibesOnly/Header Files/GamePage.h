#pragma once
#include <vector>
#include "GameObject.h"
#include "Enemy.h"

void Game_Load();
	 
void Game_Init();
	 
void Game_Update();
	 
void Game_Draw();
	 
void Game_Free();
	 
void Game_Unload();

extern std::vector<GameObject*> objects;