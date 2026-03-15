// ============================================================================
// Main.cpp - Application Entry Point
// ============================================================================
// Initialises Alpha Engine, runs the state machine game loop, and exits cleanly.
//
// GAME LOOP STRUCTURE:
// ----------------------------------------------------------------------------
// The outer loop handles state transitions (Load, Free, Unload).
// The inner loop runs while the state has not changed (Update, Draw).
//
//   OUTER LOOP (state machine):
//     Normal entry  : LoadCurrentState() -> State->Init()
//     Restart entry : RevertToPrevious() -> State->Init()  (skips Load/Unload)
//
//   INNER LOOP (per frame):
//     AESysFrameStart()
//     State->Update()
//     State->Draw()
//     AESysFrameEnd()
//
//   OUTER LOOP (state exit):
//     State->Free()
//     If not RESTART: State->Unload()
//     StateManager->Advance()  (previous = current, current = next)
//
// ESCAPE KEY BEHAVIOUR:
// ----------------------------------------------------------------------------
//   ESC exits the application ONLY when not in gameplay/pause/menu states.
//   Inside those states, ESC is handled by the state's own Update() logic.
// ============================================================================

#include "pch.hpp"
#include "StateManager.hpp"
#include "AudioManager.hpp"
#include <crtdbg.h>

// ============================================================================
// WinMain - Entry point
// ============================================================================
int APIENTRY wWinMain(_In_     HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_     LPWSTR    lpCmdLine,
    _In_     int       nCmdShow)
{
    // Enable memory leak detection in debug builds
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    // _CrtSetBreakAlloc(499); // Uncomment to break on a specific allocation number

    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // --- Engine initialisation ---
    AESysInit(hInstance, nCmdShow,
        1600, 900,   // Window width, height
        1,            // Fullscreen (0 = windowed)
        60,           // Target FPS
        false,        // Use default window proc
        NULL);

    AESysSetWindowTitle("Alpha Engine - Window");
    AESysReset();

    printf("[Main] Engine initialised.\n");

    AudioManager::Init();

    // --- State machine initialisation ---
    StateManagerInit(STATE_SPLASH);
    StateManager& sm = StateManager::Get();

    int gGameRunning = 1; // Set to 0 to exit the outer loop

    // =========================================================================
    // OUTER GAME LOOP - State transitions
    // =========================================================================
    while (gGameRunning && sm.IsRunning())
    {
        // --- State entry ---
        if (!sm.IsRestarting())
        {
            // Normal transition: destroy old state, create and load new state
            sm.LoadCurrentState();
        }
        else
        {
            // Restart: reuse existing state object (skip Load/Unload)
            sm.RevertToPrevious();
        }

        // Safety: if state creation failed (e.g. STATE_QUIT slipped through)
        if (!sm.State()) break;

        sm.State()->Init();

        // =====================================================================
        // INNER GAME LOOP - Per-frame update
        // =====================================================================
        while (sm.GetNext() == sm.GetCurrent())
        {
            AESysFrameStart();

            // ESC quits the application only outside gameplay/pause/menu states
            if (AEInputCheckTriggered(AEVK_ESCAPE) &&
                sm.GetCurrent() != STATE_PLAYING &&
                sm.GetCurrent() != STATE_PAUSE &&
                sm.GetCurrent() != STATE_MENU)
            {
                gGameRunning = 0;
            }

            // Exit if the OS window is closed
            if (!AESysDoesWindowExist())
            {
                gGameRunning = 0;

                StateManagerQuit();
            }

            // Default render state (overridden per frame by each state)
            AEGfxSetBackgroundColor(0.5f, 0.5f, 0.5f);
            AEGfxSetRenderMode(AE_GFX_RM_COLOR);
            AEGfxSetBlendMode(AE_GFX_BM_BLEND);

            sm.State()->Update();
            sm.State()->Draw();

            AESysFrameEnd();
        }

        // --- State exit ---
        sm.State()->Free();

        // Only Unload on a full state exit; skip on restart to reuse resources
        if (sm.GetNext() != STATE_RESTART)
            sm.State()->Unload();

        // Advance: previous = current, current = next
        sm.Advance();
    }
    sm.FreeManager();

    AudioManager::Free();

    // --- Engine shutdown ---
    AESysExit();

    printf("[Main] Engine shut down cleanly.\n");
    return 0;
}