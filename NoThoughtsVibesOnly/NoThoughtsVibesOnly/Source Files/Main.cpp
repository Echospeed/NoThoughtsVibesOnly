#include <crtdbg.h> // To check for memory leaks
#include "pch.h"
/* --------------------------------------------------------------------------- */
// main

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);


	int gGameRunning = 1;

	// Initialization of your own variables go here

	// Using custom window procedure
	AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, false, NULL);

	// Changing the window title
	AESysSetWindowTitle("Alpha Engine - Window");

	// reset the system modules
	AESysReset();

	printf("Start\n");

	// System Initializer (Audio, Input, Graphics, etc.)
	
	// State Manager Initialization
	StateManagerInit(STATE_SPLASH);

	f64 deltaTime = AEFrameRateControllerGetFrameTime();
	// Game Loop
	while (gGameRunning && current != STATE_QUIT)
	{
		// Informing the system about the loop's start
		if (current != STATE_RESTART)
		{
			StateManagerUpdate();
			fpLoad();
		}
		else
		{
			next = previous;
			current = next;
		}

		fpInitialize();

		//Game Loop
		while (next == current)
		{
			AESysFrameStart();
			// Movement Input
			// Basic way to trigger exiting the application
			// when ESCAPE is hit or when the window is closed
			if (AEInputCheckTriggered(AEVK_ESCAPE) || 0 == AESysDoesWindowExist())
				gGameRunning = 0;

			// Your own update logic goes here


			// Your own rendering logic goes here
			AEGfxSetBackgroundColor(0.5f, 0.5f, 0.5f);
			AEGfxSetRenderMode(AE_GFX_RM_COLOR);
			AEGfxSetBlendMode(AE_GFX_BM_BLEND);

			// Informing the system about the loop's end
			fpUpdate();
			fpDraw();
			AESysFrameEnd();
		}

		fpFree();
		if (next != STATE_RESTART)
		{
			fpUnload();
		}
		previous = current;
		current = next;
	}

	// free the system
	AESysExit();
}