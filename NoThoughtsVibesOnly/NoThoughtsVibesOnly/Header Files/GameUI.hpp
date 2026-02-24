#pragma once
#include "pch.hpp"

// Forward declarations
class Player;
class WaveSystem;
class PowerUpSystem;

// ============================================================================
// GameUI - All in-game UI drawing
// Call Init() once, then call the draw functions from Game_Draw()
// ============================================================================
class GameUI
{
public:
    // === Setup - call once in Game_Init() ===
    void Init(s8 font, WaveSystem* waves, PowerUpSystem* powerups);

    // === World space - call BEFORE AEGfxSetCamPosition(0,0) ===
    void DrawAllHealthBars();       // Health bars follow entities

    // === Screen space - call AFTER AEGfxSetCamPosition(0,0) ===
    void DrawHealthText();          // HP text
    void DrawXPBar();               // XP progress bar
    void DrawCurrentStats();        // Always-visible stat panel
    void DrawWaveInfo();            // Wave / Round / Enemy count
    void DrawWaveTimer();           // Countdown between waves
    void DrawPowerUpScreen();       // Level-up overlay

private:
    s8             gameFont{ 0 };
    WaveSystem* waveSystem{ nullptr };
    PowerUpSystem* powerUpSystem{ nullptr };

    // Helpers
    void DrawRect(f32 x, f32 y, f32 w, f32 h, f32 r, f32 g, f32 b, f32 a);
    void DrawText(const char* text, f32 x, f32 y, f32 scale, f32 r, f32 g, f32 b);

    bool IsMouseOverBox(f32 boxX, f32 boxY, f32 boxW, f32 boxH);

};