// ============================================================================
// LevelConfig.cpp - Level Configuration Implementation
// ============================================================================
#include "pch.hpp"
#include "LevelConfig.hpp"

// Default to Endless (will be overwritten by LevelSelectPage)
LevelConfig g_CurrentLevel = GetLevelConfig(LevelType::ENDLESS);
