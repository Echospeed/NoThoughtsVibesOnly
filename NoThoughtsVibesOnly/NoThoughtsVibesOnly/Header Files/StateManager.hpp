#pragma once
#include "pch.hpp"

// ============================================================================
// Screen dimensions
// ============================================================================
extern const f32 SCREEN_W;
extern const f32 SCREEN_H;

// ============================================================================
// GameState
// ============================================================================
enum GameState
{
    STATE_SPLASH = 0,
    STATE_MENU,
    STATE_PLAYING,
    STATE_PAUSE,
    STATE_FINISH,
    STATE_WIN,
    STATE_RESTART,
    STATE_QUIT
};

// ============================================================================
// IState - Abstract base class for all game states
// ============================================================================
class IState
{
public:
    virtual ~IState() = default;
    virtual void Load() = 0;
    virtual void Init() = 0;
    virtual void Update() = 0;
    virtual void Draw() = 0;
    virtual void Free() = 0;
    virtual void Unload() = 0;
};

// ============================================================================
// StateManager - Singleton
// ============================================================================
class StateManager
{
public:
    static StateManager& Get();

    void Init(GameState startState);
    void ChangeState(GameState newState);

    void LoadCurrentState();   // delete old + create + Load() new state
    void RevertToPrevious();   // RESTART: skip Load/Unload, reuse object
    void Advance();            // previous = current; current = next
	void FreeManager();           // Clean up current state (called on quit)

    GameState GetCurrent()  const { return current; }
    GameState GetPrevious() const { return previous; }
    GameState GetNext()     const { return next; }

    IState* State() const { return currentState; }

    bool IsRunning()    const { return current != STATE_QUIT; }
    bool IsRestarting() const { return current == STATE_RESTART; }

private:
    StateManager() = default;
    StateManager(const StateManager&) = delete;
    StateManager& operator=(const StateManager&) = delete;

    GameState current{ STATE_SPLASH };
    GameState previous{ STATE_SPLASH };
    GameState next{ STATE_SPLASH };

    IState* currentState{ nullptr };

    static IState* CreateState(GameState state);
};

// ============================================================================
// Free function wrappers (backward compatibility + button callbacks)
// ============================================================================
void StateManagerInit(GameState state);
void StateManagerChangeState(GameState state);
void StateManagerMenuPage();
void StateManagerGamePage();
void StateManagerQuit();