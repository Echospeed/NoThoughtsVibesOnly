#pragma once
#ifndef MAIN_MENU_H
#define MAIN_MENU_H

#include "AEEngine.h"

// The states the game can be in
enum GameState {
    STATE_MENU,
    STATE_GAME,
    STATE_QUIT
};

// Function prototypes to be called in Main.cpp
void MainMenu_Load();
void MainMenu_Update(GameState& currentState);
void MainMenu_Draw();
void MainMenu_Unload();

#endif