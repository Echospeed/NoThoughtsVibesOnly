#pragma once
#include "pch.h"
//This file acts as the central registry for your game states and declares the modular functions required by the engine's loop.
// Enumeration for Game States to handle transitions
//add new levels here

enum GameState {
	STATE_SPLASH,
	STATE_MENU,
	STATE_PLAYING,
	STATE_RESTART,
	STATE_QUIT
};

// Function Pointer Type
typedef void (*FP)(void);

// Declare variables as extern here
extern int current, previous, next;
extern FP fpLoad, fpInitialize, fpUpdate, fpDraw, fpFree, fpUnload;

// Function Prototypes
void StateManager_Init(GameState state);
void StateManager_Update();
