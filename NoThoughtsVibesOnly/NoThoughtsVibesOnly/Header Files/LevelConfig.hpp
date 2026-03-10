#pragma once
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
#pragma warning(disable: 26482) // enum used as index
#include "pch.hpp"
#pragma warning(push)
#pragma warning(disable: 26495) // uninitialized member
#pragma warning(disable: 26494) // variable not initialized
//#pragma warning(disable: 26482) // enum used as index
#pragma warning(disable: 26451) // arithmetic overflow checks
#include "include/rapidjson/document.h"
#include "include/rapidjson/filereadstream.h"
#pragma warning(pop)
#include <cstdio>
#include <string>
#include <iostream>

enum class LevelType
{
    LEVEL_1,
    LEVEL_2,
    ENDLESS
};

struct LevelConfig
{
    LevelType   type;
    u32         numWaves;
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
inline bool LoadLevelFromJson(const char* filepath, LevelConfig& cfg)
{
    FILE* fp = fopen(filepath, "rb");
    if (!fp)
    {
        std::cout << "[LevelConfig] WARNING: Could not open '" << filepath << "' - using defaults.\n";
        return false;
    }

    char buf[4096]{};
    rapidjson::FileReadStream is(fp, buf, sizeof(buf));

    rapidjson::Document doc;
    doc.ParseStream(is);
    fclose(fp);

    if (doc.HasParseError())
    {
        std::cout << "[LevelConfig] WARNING: Parse error in '" << filepath << "' - using defaults.\n";
        return false;
    }

    // Read each field with a safe HasMember check before accessing
    if (doc.HasMember("numWaves") && doc["numWaves"].IsInt())
        cfg.numWaves = (u32)doc["numWaves"].GetInt();

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

    std::cout << "[LevelConfig] Loaded '" << cfg.name << "' from " << filepath << "\n";
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
    return { LevelType::ENDLESS, UINT32_MAX, true, 3.0f, 5, 20, "Endless", "Infinite waves. Survive as long as you can." };
}