#pragma once
// Author: John Chiow
// Co-Author: Stanley Lu
#include "pch.hpp"

// Forward declarations
class Player;
class WaveSystem;
class PowerUpSystem;

// ============================================================================
// 
// - All in-game UI drawing
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
    void DrawAbilities();           // Abilities display
    void ToggleStats() { m_ShowStats = !m_ShowStats; }

private:
    s8 gameFont{ 0 };
    WaveSystem* waveSystem{ nullptr };
    PowerUpSystem* powerUpSystem{ nullptr };
    bool m_ShowStats{ true };

    // Text renderers
    TextRenderer playerHealth;
    TextRenderer playerXP;
    TextRenderer playerStats;
    TextRenderer waveInfo;
    TextRenderer waveRound;
    TextRenderer waveTimer;
    TextRenderer activeEnemyCount;
    TextRenderer powerUpTitle;
    TextRenderer statBuffText;
    TextRenderer descriptionText;
    TextRenderer abilityTimerText;
    TextRenderer abilityKeyText;

    // Helpers
    void DrawRect(f32 x, f32 y, f32 w, f32 h, f32 r, f32 g, f32 b, f32 a);

    //bool IsMouseOverBox(f32 boxX, f32 boxY, f32 boxW, f32 boxH);
};