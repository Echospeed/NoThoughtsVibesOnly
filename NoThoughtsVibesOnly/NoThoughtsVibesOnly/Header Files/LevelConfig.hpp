#pragma once
#include "pch.hpp"

enum class LevelType
{ 
	LEVEL_1,
	LEVEL_2,
	ENDLESS
};

struct LevelConfig
{
	LevelType type;
	u32 numWaves;
	bool hasBoss;
};

extern LevelConfig g_CurrentLevel;

inline LevelConfig GetLevelConfig(LevelType levelType)
{
	switch (levelType)
	{
	case LevelType::LEVEL_1:
		return { LevelType::LEVEL_1, 5, true };
	case LevelType::LEVEL_2:
		return { LevelType::LEVEL_2, 10, true };
	case LevelType::ENDLESS:
		return { LevelType::ENDLESS, UINT32_MAX, true };
	default:
		return { LevelType::ENDLESS, 5, false }; // Default to Endless config
	}
}
