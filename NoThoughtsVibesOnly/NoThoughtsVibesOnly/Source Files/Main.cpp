#include "pch.hpp"
#include "StateManager.hpp"
#include <crtdbg.h>

/* --------------------------------------------------------------------------- */
// main
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(499);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	int gGameRunning = 1;

	AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, false, NULL);
	AESysSetWindowTitle("Alpha Engine - Window");
	AESysReset();

	printf("Start\n");

	StateManagerInit(STATE_SPLASH);

	StateManager& sm = StateManager::Get();

	// Game Loop
	while (gGameRunning && sm.IsRunning())
	{
		if (!sm.IsRestarting())
		{
			// Normal transition: create new IState object and call Load()
			sm.LoadCurrentState();
		}
		else
		{
			// RESTART: revert current/next back to previous, reuse existing state object
			sm.RevertToPrevious();
		}

		// Guard: if state object failed to create (STATE_QUIT slipping through etc.)
		if (!sm.State()) break;

		sm.State()->Init();

		// Inner loop - runs while state has not changed
		while (sm.GetNext() == sm.GetCurrent())
		{
			AESysFrameStart();

			// Escape quits only outside gameplay / pause / menu
			if (AEInputCheckTriggered(AEVK_ESCAPE) &&
				sm.GetCurrent() != STATE_PLAYING &&
				sm.GetCurrent() != STATE_PAUSE &&
				sm.GetCurrent() != STATE_MENU)
			{
				gGameRunning = 0;
			}

			if (0 == AESysDoesWindowExist())
				gGameRunning = 0;

			AEGfxSetBackgroundColor(0.5f, 0.5f, 0.5f);
			AEGfxSetRenderMode(AE_GFX_RM_COLOR);
			AEGfxSetBlendMode(AE_GFX_BM_BLEND);

			sm.State()->Update();
			sm.State()->Draw();

			AESysFrameEnd();
		}

		sm.State()->Free();

		// Check NEXT (not current) to decide whether to Unload
		// At this point current = old state, next = requested new state
		if (sm.GetNext() != STATE_RESTART)
		{
			sm.State()->Unload();
		}

		// Advance: previous = current; current = next
		sm.Advance();
	}

	AESysExit();
}