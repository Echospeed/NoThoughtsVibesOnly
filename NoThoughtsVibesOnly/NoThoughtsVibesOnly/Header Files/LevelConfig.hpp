#pragma once
// Author: John Chiow
// ============================================================================
// LevelConfig.hpp - Level Configuration (RapidJSON-backed)
// ============================================================================
// LevelConfig holds the settings for a single level mode.
// Values are loaded from JSON files in Assets/ at runtime so designers can
// tweak wave counts, boss flags, etc. without recompiling.
//
// JSON FILES:
// ----------------------------------------------------------------------------
//   Assets/level1.json  -> LevelType::LEVEL_1
//   Assets/level2.json  -> LevelType::LEVEL_2
//   Endless mode has no JSON file - it uses hardcoded infinite defaults.
//
// USAGE:
// ----------------------------------------------------------------------------
//   // In LevelSelectPage - set the global before entering STATE_PLAYING:
//   g_CurrentLevel = GetLevelConfig(LevelType::LEVEL_1);
//
//   // In WaveSystem - read config values:
//   u32   waves     = g_CurrentLevel.numWaves;
//   bool  boss      = g_CurrentLevel.hasBoss;
//   float breakTime = g_CurrentLevel.waveBreakDuration;
// ============================================================================
#include "pch.hpp"

// RapidJSON includes (Warning suppressions removed by request)
#include "include/rapidjson/document.h"
#include "include/rapidjson/istreamwrapper.h"

#include <fstream> // Using modern C++ file streams
#include <string>
#include <iostream>
#include <cstdint> // Required for cross-platform fixed-width types like uint32_t

// Strongly typed scoped enum for level selection
enum class LevelType
{
    LEVEL_1,
    LEVEL_2,
    ENDLESS
};

struct LevelConfig
{
    LevelType   type;
    uint32_t    numWaves;
    bool        hasBoss;
    float       waveBreakDuration; // Seconds between waves
    int         baseEnemyCount;    // Starting enemy count per wave
    int         maxEnemyCount;     // Cap on enemies per wave
    std::string name;              // Display name e.g. "Level 1"
    std::string description;       // Shown on level select screen
};

extern LevelConfig g_CurrentLevel;

// ============================================================================
// LoadLevelFromJson (internal helper)
// ============================================================================
// Opens filepath, parses it with RapidJSON, and fills out cfg.
// Returns true on success, false if file missing or parse error.
// ============================================================================
inline bool LoadLevelFromJson(const std::string& filepath, LevelConfig& cfg)
{
    // C++ style file handling
    std::ifstream ifs(filepath);
    if (!ifs.is_open())
    {
        //std::cout << "[LevelConfig] WARNING: Could not open '" << filepath << "' - using defaults.\n";
        return false;
    }

    // Bridge standard streams to RapidJSON
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document doc;
    doc.ParseStream(isw);

    if (doc.HasParseError())
    {
        //std::cout << "[LevelConfig] WARNING: Parse error in '" << filepath << "' - using defaults.\n";
        return false;
    }

    // Read each field with a safe HasMember check before accessing.
    // Explicit static_cast guarantees the JSON Int matches our uint32_t expectation.
    if (doc.HasMember("numWaves") && doc["numWaves"].IsInt())
        cfg.numWaves = static_cast<unsigned int>(doc["numWaves"].GetInt());

    if (doc.HasMember("hasBoss") && doc["hasBoss"].IsBool())
        cfg.hasBoss = doc["hasBoss"].GetBool();

    if (doc.HasMember("waveBreakDuration") && doc["waveBreakDuration"].IsNumber())
        cfg.waveBreakDuration = doc["waveBreakDuration"].GetFloat();

    if (doc.HasMember("baseEnemyCount") && doc["baseEnemyCount"].IsInt())
        cfg.baseEnemyCount = doc["baseEnemyCount"].GetInt();

    if (doc.HasMember("maxEnemyCount") && doc["maxEnemyCount"].IsInt())
        cfg.maxEnemyCount = doc["maxEnemyCount"].GetInt();

    if (doc.HasMember("name") && doc["name"].IsString())
        cfg.name = doc["name"].GetString();

    if (doc.HasMember("description") && doc["description"].IsString())
        cfg.description = doc["description"].GetString();

    //std::cout << "[LevelConfig] Loaded '" << cfg.name << "' from " << filepath << "\n";
    return true;
}

// ============================================================================
// GetLevelConfig
// ============================================================================
// Returns the LevelConfig for the given type.
// Tries to load from JSON first; falls back to hardcoded defaults if missing.
// ============================================================================
inline LevelConfig GetLevelConfig(LevelType levelType)
{
    if (levelType == LevelType::LEVEL_1)
    {
        LevelConfig cfg{ LevelType::LEVEL_1, 5, false, 3.0f, 5, 20, "Level 1", "5 waves, no boss." };
        LoadLevelFromJson("Assets/level1.json", cfg);
        return cfg;
    }

    if (levelType == LevelType::LEVEL_2)
    {
        LevelConfig cfg{ LevelType::LEVEL_2, 10, true, 3.0f, 5, 20, "Level 2", "10 waves, boss on final wave." };
        LoadLevelFromJson("Assets/level2.json", cfg);
        return cfg;
    }

    // Endless - no JSON, always hardcoded
    // Leverages UINT32_MAX to mathematically represent infinite waves safely
    return { LevelType::ENDLESS, UINT32_MAX, true, 3.0f, 5, 20, "Endless", "Infinite waves. Survive as long as you can." };
}