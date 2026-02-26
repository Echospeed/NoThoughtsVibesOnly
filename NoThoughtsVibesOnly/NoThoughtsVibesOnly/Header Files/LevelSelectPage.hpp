// ============================================================================
// LevelSelectPage.hpp - Level Selection Screen
// ============================================================================
// Allows the player to choose between Level 1, Level 2, or Endless mode.
// ============================================================================

#pragma once
#include "StateManager.hpp"

void LevelSelect_Load();
void LevelSelect_Init();
void LevelSelect_Update();
void LevelSelect_Draw();
void LevelSelect_Free();
void LevelSelect_Unload();

extern std::vector<GameObject*> levelSelectPageObj;
