// Author: John Chiow

// ============================================================================
// LevelConfig.cpp - Level Configuration Implementation
// ============================================================================
#include "pch.hpp"
#include "LevelConfig.hpp"

// Default to Endless (will be overwritten by LevelSelectPage)
// Instantiates the global config block used throughout the system
LevelConfig g_CurrentLevel = GetLevelConfig(LevelType::ENDLESS);