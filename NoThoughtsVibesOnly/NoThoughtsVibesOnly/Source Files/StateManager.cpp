#include "pch.hpp"
#include "MenuPage.hpp"
#include "GamePage.hpp"
#include "SplashPage.hpp"
#include "FinishPage.hpp"
#include "PausePage.hpp"
#include "WinPage.hpp"

// ============================================================================
// Screen dimensions
// ============================================================================
const f32 SCREEN_W = 1600.0f;
const f32 SCREEN_H = 900.0f;

// ============================================================================
// Concrete IState wrappers - delegate to each page's free functions
// ============================================================================
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

// ============================================================================
// StateManager - Singleton
// ============================================================================
StateManager& StateManager::Get()
{
    static StateManager instance;
    return instance;
}

void StateManager::Init(GameState startState)
{
    current = previous = next = startState;
    currentState = nullptr;
}

void StateManager::ChangeState(GameState newState)
{
    next = newState;
}

IState* StateManager::CreateState(GameState state)
{
    switch (state)
    {
    case STATE_SPLASH:  return new SplashState();
    case STATE_MENU:    return new MenuState();
    case STATE_PLAYING: return new PlayingState();
    case STATE_PAUSE:   return new PauseState();
    case STATE_FINISH:  return new FinishState();
    case STATE_WIN:     return new WinState();
    default:            return nullptr;
    }
}

void StateManager::LoadCurrentState()
{
    printf("StateManager::LoadCurrentState: State = %d\n", current);
    delete currentState;
    currentState = CreateState(current);
    if (currentState)
        currentState->Load();
}

void StateManager::RevertToPrevious()
{
    next = previous;
    current = next;
}

void StateManager::Advance()
{
    previous = current;
    current = next;
}

// ============================================================================
// Free function wrappers
// ============================================================================
void StateManagerInit(GameState state) { StateManager::Get().Init(state); }
void StateManagerChangeState(GameState state) { StateManager::Get().ChangeState(state); }
void StateManagerMenuPage() { StateManager::Get().ChangeState(STATE_MENU); }
void StateManagerGamePage() { StateManager::Get().ChangeState(STATE_PLAYING); }
void StateManagerQuit() { StateManager::Get().ChangeState(STATE_QUIT); }