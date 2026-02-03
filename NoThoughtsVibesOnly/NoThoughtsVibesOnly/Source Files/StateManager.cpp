#include "pch.h"
#include "StateManager.h"
#include "MenuPage.h"
#include "GamePage.h"
#include "SplashPage.h"
#include "FinishPage.h"

int current = 0, previous = 0, next = 0;

FP fpLoad = nullptr, fpInitialize = nullptr, fpUpdate = nullptr, fpDraw = nullptr, fpFree = nullptr, fpUnload = nullptr;

const f32 SCREEN_W = 1600.0f;
const f32 SCREEN_H = 900.0f;

void StateManagerInit(GameState state)
{
	next = previous = current = state;
}

void StateManagerUpdate()
{
	printf("StateManager_Update: Current State = %d\n", current);

	switch (current)
	{
		case STATE_SPLASH:
			fpLoad = SplashPage_Load;
			fpInitialize = SplashPage_Init;
			fpUpdate = SplashPage_Update;
			fpDraw = SplashPage_Draw;
			fpFree = SplashPage_Free;
			fpUnload = SplashPage_Unload;
			break;
		case STATE_MENU:
			fpLoad = Main_Load;
			fpInitialize = Main_Init;
			fpUpdate = Main_Update;
			fpDraw = Main_Draw;
			fpFree = Main_Free;
			fpUnload = Main_Unload;
			break;
		case STATE_PLAYING:
			fpLoad = Game_Load;
			fpInitialize = Game_Init;
			fpUpdate = Game_Update;
			fpDraw = Game_Draw;
			fpFree = Game_Free;
			fpUnload = Game_Unload;
			break;
		case STATE_FINISH:
			fpLoad = FinishPage_Load;
			fpInitialize = FinishPage_Init;
			fpUpdate = FinishPage_Update;
			fpDraw = FinishPage_Draw;
			fpFree = FinishPage_Free;
			fpUnload = FinishPage_Unload;
			break;
		case STATE_WIN:
			fpLoad = WinPage_Load;
			fpInitialize = WinPage_Init;
			fpUpdate = WinPage_Update;
			fpDraw = WinPage_Draw;
			fpFree = WinPage_Free;
			fpUnload = WinPage_Unload;
			break;
		case STATE_RESTART:
			// Handled in Main.cpp game loop
			break;
		case STATE_QUIT:
			// Handled in Main.cpp game loop
			break;
		default:
			break;
	}
}

void StateManagerChangeState(GameState state)
{
	next = state;
}

void StateManagerMenuPage()
{
	StateManagerChangeState(STATE_MENU);
}

void StateManagerGamePage()
{
	StateManagerChangeState(STATE_PLAYING);
}

void StateManagerQuit()
{
	StateManagerChangeState(STATE_QUIT);
}