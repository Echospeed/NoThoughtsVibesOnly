#pragma once
#include "pch.h"

enum GameState 
{
	STATE_MENU,
	STATE_PLAYING,
	STATE_RESTART,
	STATE_QUIT
};

typedef void(*FP)(void);

extern int current, previous, next;

extern FP fpLoad, fpInitialize, fpUpdate, fpDraw, fpFree, fpUnload;

void StateManager_Init(GameState state);

void StateManager_Update();

