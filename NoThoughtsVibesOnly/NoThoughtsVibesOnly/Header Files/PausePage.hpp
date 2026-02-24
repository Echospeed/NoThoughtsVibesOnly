// ============================================================================
// PausePage.hpp - COPY AND PASTE THIS ENTIRE FILE
// ============================================================================
// NEW FEATURE: Pause Screen System
// Create this as a NEW file in your project
// 
// Features:
//   - Pause/unpause with ESC key
//   - Resume button
//   - Restart button
//   - Main menu button
//   - Semi-transparent overlay
//   - Game freezes while paused
// ============================================================================

#pragma once

// ============================================================================
// Pause Page State Functions
// ============================================================================
// These functions manage the pause screen lifecycle
// Called by StateManager when entering/exiting pause state
// ============================================================================

void PausePage_Load();      // Load resources (fonts, textures)
void PausePage_Init();       // Initialize pause screen (reset state)
void PausePage_Update();     // Update pause screen logic (button clicks)
void PausePage_Draw();       // Draw pause overlay and buttons
void PausePage_Free();       // Clean up pause screen state
void PausePage_Unload();     // Unload resources (fonts, textures)

// ============================================================================
// Pause System Functions
// ============================================================================
// Use these to control pausing from anywhere in your game
// ============================================================================

// Check if game is currently paused
bool IsPaused();

// Pause the game
void PauseGame();

// Unpause the game
void UnpauseGame();

// Toggle pause state
void TogglePause();
