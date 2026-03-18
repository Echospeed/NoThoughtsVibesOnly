#include "pch.hpp"
// ============================================================================
// Leaderboard.cpp - Top-10 Score Tracking Implementation (RapidJSON)
// ============================================================================
// Uses RapidJSON for all file I/O.
// Scores are kept sorted highest-first and capped at MAX_ENTRIES (10).
//
// NAME FILTER:
// ----------------------------------------------------------------------------
//   LoadBlocklist() reads Assets/blocklist.txt once (one word per line).
//   IsNameBlocked() lowercases the input and checks if any blocked word
//   appears as a substring - catches words hidden inside longer names.
//   Submit() calls IsNameBlocked() and returns false if the name is rejected.
//
// JSON STRUCTURE written by Save():
// ----------------------------------------------------------------------------
//   {
//     "entries": [
//       { "name": "Player", "score": 1000, "level": "Level 1", "wave": 5 },
//       { "name": "Player", "score":  800, "level": "Level 2", "wave": 3 }
//     ]
//   }
// ============================================================================
#include "Leaderboard.hpp"

#include "include/rapidjson/document.h"
#include "include/rapidjson/istreamwrapper.h"
#include "include/rapidjson/ostreamwrapper.h"
#include "include/rapidjson/prettywriter.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>

namespace Leaderboard
{
    static std::vector<LeaderboardEntry> s_Entries;
    static bool s_Loaded = false;

    // ========================================================================
    // Blocklist
    // ========================================================================
    static std::vector<std::string> s_Blocklist;
    static bool s_BlocklistLoaded = false;

    // Loads Assets/blocklist.txt into s_Blocklist.
    // Each non-empty line is treated as one banned word (stored lowercase).
    // Called automatically the first time IsNameBlocked() runs.
    static void LoadBlocklist()
    {
        s_BlocklistLoaded = true; // Set first so a missing file doesn't retry every frame
        s_Blocklist.clear();

        std::ifstream ifs(BLOCKLIST_PATH);
        if (!ifs.is_open())
        {
            std::cout << "[Leaderboard] WARNING: '" << BLOCKLIST_PATH
                << "' not found - name filter is disabled.\n";
            return;
        }

        std::string line;
        while (std::getline(ifs, line))
        {
            // Trim trailing carriage return (Windows line endings)
            if (!line.empty() && line.back() == '\r')
                line.pop_back();

            // Lowercase the word so comparison is case-insensitive
            std::transform(line.begin(), line.end(), line.begin(),
                [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

            if (!line.empty())
                s_Blocklist.push_back(line);
        }

        std::cout << "[Leaderboard] Blocklist loaded: "
            << s_Blocklist.size() << " entries from " << BLOCKLIST_PATH << "\n";
    }

    // ========================================================================
    // IsNameBlocked
    // ========================================================================
    // Returns true if 'name' contains any blocked word as a substring.
    // The check is fully case-insensitive.
    // ========================================================================
    bool IsNameBlocked(const std::string& name)
    {
        if (!s_BlocklistLoaded) LoadBlocklist();
        if (s_Blocklist.empty()) return false;

        // Lowercase copy of the submitted name for comparison
        std::string lower = name;
        std::transform(lower.begin(), lower.end(), lower.begin(),
            [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        for (const auto& blocked : s_Blocklist)
        {
            if (lower.find(blocked) != std::string::npos)
            {
                std::cout << "[Leaderboard] Name '" << name
                    << "' rejected - matched blocked word.\n";
                return true;
            }
        }
        return false;
    }

    // ========================================================================
    // Load
    // ========================================================================
    void Load()
    {
        s_Entries.clear();

        std::ifstream ifs(SAVE_PATH);
        if (!ifs.is_open())
        {
            std::cout << "[Leaderboard] No save file found - starting fresh.\n";
            s_Loaded = true;
            return;
        }

        rapidjson::IStreamWrapper isw(ifs);
        rapidjson::Document doc;
        doc.ParseStream(isw);

        if (doc.HasParseError() || !doc.HasMember("entries") || !doc["entries"].IsArray())
        {
            std::cout << "[Leaderboard] WARNING: Corrupt save file - starting fresh.\n";
            s_Loaded = true;
            return;
        }

        const auto& arr = doc["entries"].GetArray();
        for (rapidjson::SizeType i = 0; i < arr.Size() && static_cast<int>(i) < MAX_ENTRIES; ++i)
        {
            const auto& obj = arr[i];
            LeaderboardEntry e;

            e.name = obj.HasMember("name") && obj["name"].IsString() ? obj["name"].GetString() : "Player";
            e.score = obj.HasMember("score") && obj["score"].IsInt() ? obj["score"].GetInt() : 0;
            e.level = obj.HasMember("level") && obj["level"].IsString() ? obj["level"].GetString() : "Unknown";
            e.wave = obj.HasMember("wave") && obj["wave"].IsInt() ? obj["wave"].GetInt() : 0;

            s_Entries.push_back(e);
        }

        std::sort(s_Entries.begin(), s_Entries.end(),
            [](const LeaderboardEntry& a, const LeaderboardEntry& b)
            { return a.score > b.score; });

        s_Loaded = true;
        std::cout << "[Leaderboard] Loaded " << s_Entries.size()
            << " entries from " << SAVE_PATH << "\n";
    }

    // ========================================================================
    // Save
    // ========================================================================
    void Save()
    {
        std::ofstream ofs(SAVE_PATH);
        if (!ofs.is_open())
        {
            std::cout << "[Leaderboard] ERROR: Could not write to " << SAVE_PATH << "\n";
            return;
        }

        rapidjson::Document doc;
        doc.SetObject();
        auto& alloc = doc.GetAllocator();

        rapidjson::Value arr(rapidjson::kArrayType);

        for (const auto& e : s_Entries)
        {
            rapidjson::Value obj(rapidjson::kObjectType);

            rapidjson::Value name(e.name.c_str(), static_cast<rapidjson::SizeType>(e.name.size()), alloc);
            rapidjson::Value level(e.level.c_str(), static_cast<rapidjson::SizeType>(e.level.size()), alloc);

            obj.AddMember("name", name, alloc);
            obj.AddMember("score", e.score, alloc);
            obj.AddMember("level", level, alloc);
            obj.AddMember("wave", e.wave, alloc);

            arr.PushBack(obj, alloc);
        }

        doc.AddMember("entries", arr, alloc);

        rapidjson::OStreamWrapper osw(ofs);
        rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
        doc.Accept(writer);

        std::cout << "[Leaderboard] Saved " << s_Entries.size()
            << " entries to " << SAVE_PATH << "\n";
    }

    // ========================================================================
    // Submit
    // ========================================================================
    // Returns false (and does NOT save) if the name is blocked.
    // Returns true if the name is clean and the score was saved.
    // ========================================================================
    bool Submit(const std::string& name, int score,
        const std::string& levelName, int waveReached)
    {
        if (!s_Loaded) Load();

        // --- Name filter ---
        if (IsNameBlocked(name))
            return false; // Caller should show an error message to the player

        LeaderboardEntry e;
        e.name = name.empty() ? "Player" : name;
        e.score = score;
        e.level = levelName;
        e.wave = waveReached;

        s_Entries.push_back(e);

        std::sort(s_Entries.begin(), s_Entries.end(),
            [](const LeaderboardEntry& a, const LeaderboardEntry& b)
            { return a.score > b.score; });

        if (static_cast<int>(s_Entries.size()) > MAX_ENTRIES)
            s_Entries.resize(MAX_ENTRIES);

        std::cout << "[Leaderboard] New entry: " << e.name
            << " | " << e.score << " pts"
            << " | " << e.level
            << " | Wave " << e.wave << "\n";

        Save();
        return true;
    }

    // ========================================================================
    // GetEntries
    // ========================================================================
    const std::vector<LeaderboardEntry>& GetEntries()
    {
        if (!s_Loaded) Load();
        return s_Entries;
    }

    // ========================================================================
    // Clear
    // ========================================================================
    void Clear()
    {
        s_Entries.clear();
        Save();
        std::cout << "[Leaderboard] Cleared.\n";
    }

} // namespace Leaderboard