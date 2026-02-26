// ============================================================================
// StateManager.cpp - Game State Machine Implementation
// ============================================================================
// Manages the game's page/state lifecycle using the IState interface.
// Each page (Splash, Menu, Game, Pause, Finish, Win) is wrapped in a
// concrete IState subclass that delegates to the page's free functions.
//
// STATE LIFECYCLE (per IState):
// ----------------------------------------------------------------------------
//   Load()   : Allocate GPU resources (textures, meshes, fonts). Called once.
//   Init()   : Initialise runtime data.   Called each entry (inc. restart).
//   Update() : Per-frame logic.
//   Draw()   : Per-frame rendering.
//   Free()   : Destroy runtime data.      Called each exit.
//   Unload() : Release GPU resources.     NOT called on STATE_RESTART.
//
// STATE TRANSITIONS:
// ----------------------------------------------------------------------------
//   Normal    : Free -> Unload -> new state -> Load -> Init
//   Restart   : Free -> (skip Unload) -> (reuse state object) -> Init
//   Quit      : Exits the main loop.
//
// TO ADD A NEW STATE:
// ----------------------------------------------------------------------------
//   1. Add an enum value to GameState (StateManager.hpp).
//   2. Create a concrete IState subclass here.
//   3. Add a case to CreateState().
// ============================================================================

#include "pch.hpp"
#include "MenuPage.hpp"
#include "GamePage.hpp"
#include "SplashPage.hpp"
#include "FinishPage.hpp"
#include "PausePage.hpp"
#include "WinPage.hpp"
#include "LevelSelectPage.hpp"

// ============================================================================
// Screen dimensions (defined here, declared extern in StateManager.hpp)
// ============================================================================
const f32 SCREEN_W = 1600.0f;
const f32 SCREEN_H = 900.0f;

// ============================================================================
// Concrete IState wrappers
// ============================================================================
// Each class delegates all lifecycle calls to the corresponding page's
// free functions, keeping the state machine decoupled from page logic.

class SplashState final : public IState
{
public:
    void Load()   override { SplashPage_Load(); }
    void Init()   override { SplashPage_Init(); }
    void Update() override { SplashPage_Update(); }
    void Draw()   override { SplashPage_Draw(); }
    void Free()   override { SplashPage_Free(); }
    void Unload() override { SplashPage_Unload(); }
};

class MenuState final : public IState
{
public:
    void Load()   override { Main_Load(); }
    void Init()   override { Main_Init(); }
    void Update() override { Main_Update(); }
    void Draw()   override { Main_Draw(); }
    void Free()   override { Main_Free(); }
    void Unload() override { Main_Unload(); }
};

class PlayingState final : public IState
{
public:
    void Load()   override { Game_Load(); }
    void Init()   override { Game_Init(); }
    void Update() override { Game_Update(); }
    void Draw()   override { Game_Draw(); }
    void Free()   override { Game_Free(); }
    void Unload() override { Game_Unload(); }
};

class PauseState final : public IState
{
public:
    void Load()   override { PausePage_Load(); }
    void Init()   override { PausePage_Init(); }
    void Update() override { PausePage_Update(); }
    void Draw()   override { PausePage_Draw(); }
    void Free()   override { PausePage_Free(); }
    void Unload() override { PausePage_Unload(); }
};

class FinishState final : public IState
{
public:
    void Load()   override { FinishPage_Load(); }
    void Init()   override { FinishPage_Init(); }
    void Update() override { FinishPage_Update(); }
    void Draw()   override { FinishPage_Draw(); }
    void Free()   override { FinishPage_Free(); }
    void Unload() override { FinishPage_Unload(); }
};

class WinState final : public IState
{
public:
    void Load()   override { WinPage_Load(); }
    void Init()   override { WinPage_Init(); }
    void Update() override { WinPage_Update(); }
    void Draw()   override { WinPage_Draw(); }
    void Free()   override { WinPage_Free(); }
    void Unload() override { WinPage_Unload(); }
};

class LevelSelectState final : public IState
{
public:
    void Load()   override { LevelSelect_Load(); }
    void Init()   override { LevelSelect_Init(); }
    void Update() override { LevelSelect_Update(); }
    void Draw()   override { LevelSelect_Draw(); }
    void Free()   override { LevelSelect_Free(); }
    void Unload() override { LevelSelect_Unload(); }
};

// ============================================================================
// StateManager - Singleton Implementation
// ============================================================================

StateManager& StateManager::Get()
{
    static StateManager instance;
    return instance;
}

// ============================================================================
// Init
// ============================================================================
// Sets the initial state. Call once in main() before the game loop.
// ============================================================================
void StateManager::Init(GameState startState)
{
    current = previous = next = startState;
    currentState = nullptr;
}

// ============================================================================
// ChangeState
// ============================================================================
// Requests a transition to newState. The transition happens at the start
// of the next outer game loop iteration.
// ============================================================================
void StateManager::ChangeState(GameState newState)
{
    next = newState;
}

// ============================================================================
// CreateState (private static)
// ============================================================================
// Factory: returns a new heap-allocated IState for the given GameState.
// Returns nullptr for unknown or terminal states (STATE_QUIT, STATE_RESTART).
// ============================================================================
IState* StateManager::CreateState(GameState state)
{
    switch (state)
    {
    case STATE_SPLASH:       return new SplashState();
    case STATE_MENU:         return new MenuState();
    case STATE_LEVEL_SELECT: return new LevelSelectState();
    case STATE_PLAYING:      return new PlayingState();
    case STATE_PAUSE:        return new PauseState();
    case STATE_FINISH:       return new FinishState();
    case STATE_WIN:          return new WinState();
    default:                 return nullptr;
    }
}

// ============================================================================
// LoadCurrentState
// ============================================================================
// Destroys the current state object, creates and loads the new one.
// Called for normal (non-restart) transitions.
// ============================================================================
void StateManager::LoadCurrentState()
{
    printf("[StateManager] Loading state %d\n", current);

    delete currentState;
    currentState = CreateState(current);

    if (currentState) currentState->Load();
}

// ============================================================================
// RevertToPrevious
// ============================================================================
// For RESTART: skips Load/Unload and reuses the existing state object.
// Sets next = previous = current so the state machine re-enters the inner loop.
// ============================================================================
void StateManager::RevertToPrevious()
{
    next = previous;
    current = next;
}

// ============================================================================
// Advance
// ============================================================================
// Called after Free()/Unload() to move to the next state.
// ============================================================================
void StateManager::Advance()
{
    previous = current;
    current = next;
}

void StateManager::FreeManager()
{
    delete currentState;
    currentState = nullptr;
}

// ============================================================================
// Free function wrappers
// ============================================================================
void StateManagerInit(GameState state) { StateManager::Get().Init(state); }
void StateManagerChangeState(GameState state) { StateManager::Get().ChangeState(state); }
void StateManagerMenuPage() { StateManager::Get().ChangeState(STATE_MENU); }
void StateManagerGamePage() { StateManager::Get().ChangeState(STATE_PLAYING); }
void StateManagerQuit() { StateManager::Get().ChangeState(STATE_QUIT); }