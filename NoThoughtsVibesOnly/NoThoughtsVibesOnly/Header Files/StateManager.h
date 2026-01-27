#pragma once
#include "pch.h"
//This file acts as the central registry for your game states and declares the modular functions required by the engine's loop.
// Enumeration for Game States to handle transitions
//add new levels here

enum GameState 
{
	STATE_SPLASH,
	STATE_MENU,
	STATE_PLAYING,
	STATE_LOSE,
	STATE_RESTART,
	STATE_QUIT
};

typedef void(*FP)(void);

extern int current, previous, next;

extern FP fpLoad, fpInitialize, fpUpdate, fpDraw, fpFree, fpUnload;

extern const f32 SCREEN_W, SCREEN_H;

void StateManagerInit(GameState state);

void StateManagerUpdate();

void StateManagerChangeState(GameState state);

void StateManagerMenuPage();

void StateManagerGamePage();

void StateManagerQuit();