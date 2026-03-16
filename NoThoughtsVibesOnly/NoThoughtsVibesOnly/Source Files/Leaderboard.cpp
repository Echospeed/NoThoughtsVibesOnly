#include "pch.hpp"
// ============================================================================
// Leaderboard.cpp - Top-10 Score Tracking Implementation (RapidJSON)
// ============================================================================
// Uses RapidJSON for all file I/O.
// Scores are kept sorted highest-first and capped at MAX_ENTRIES (10).
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

// RapidJSON includes (Warning suppressions removed by request)
#include "include/rapidjson/document.h"
#include "include/rapidjson/istreamwrapper.h"
#include "include/rapidjson/ostreamwrapper.h"
#include "include/rapidjson/prettywriter.h"

#include <algorithm>
#include <iostream>
#include <fstream> // Using modern C++ file streams

namespace Leaderboard
{
    static std::vector<LeaderboardEntry> s_Entries;
    static bool s_Loaded = false;

    // ========================================================================
    // Load
    // ========================================================================
    void Load()
    {
        s_Entries.clear();

        // Using std::ifstream for safe C++ file reading
        std::ifstream ifs(SAVE_PATH);
        if (!ifs.is_open())
        {
            std::cout << "[Leaderboard] No save file found - starting fresh.\n";
            s_Loaded = true;
            return;
        }

        // Wrap the standard input stream for RapidJSON to process
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

        // Ensure sorted order after load using the C++ standard algorithm
        std::sort(s_Entries.begin(), s_Entries.end(),
            [](const LeaderboardEntry& a, const LeaderboardEntry& b)
            { return a.score > b.score; });

        s_Loaded = true;
        std::cout << "[Leaderboard] Loaded " << s_Entries.size() << " entries from " << SAVE_PATH << "\n";
    }

    // ========================================================================
    // Save
    // ========================================================================
    void Save()
    {
        // Using std::ofstream for safe C++ file writing
        std::ofstream ofs(SAVE_PATH);
        if (!ofs.is_open())
        {
            std::cout << "[Leaderboard] ERROR: Could not write to " << SAVE_PATH << "\n";
            return;
        }

        // Build the JSON document
        rapidjson::Document doc;
        doc.SetObject();
        auto& alloc = doc.GetAllocator();

        rapidjson::Value arr(rapidjson::kArrayType);

        for (const auto& e : s_Entries)
        {
            rapidjson::Value obj(rapidjson::kObjectType);

            // rapidjson::StringRef requires the string to outlive the document,
            // so we copy strings using the allocator.
            // Using static_cast to safely match RapidJSON's expected SizeType.
            rapidjson::Value name(e.name.c_str(), static_cast<rapidjson::SizeType>(e.name.size()), alloc);
            rapidjson::Value level(e.level.c_str(), static_cast<rapidjson::SizeType>(e.level.size()), alloc);

            obj.AddMember("name", name, alloc);
            obj.AddMember("score", e.score, alloc);
            obj.AddMember("level", level, alloc);
            obj.AddMember("wave", e.wave, alloc);

            arr.PushBack(obj, alloc);
        }

        doc.AddMember("entries", arr, alloc);

        // Write with pretty formatting so the file is human-readable.
        // OStreamWrapper bridges RapidJSON with std::ofstream.
        rapidjson::OStreamWrapper osw(ofs);
        rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
        doc.Accept(writer);

        std::cout << "[Leaderboard] Saved " << s_Entries.size() << " entries to " << SAVE_PATH << "\n";
    }

    // ========================================================================
    // Submit
    // ========================================================================
    void Submit(const std::string& name, int score,
        const std::string& levelName, int waveReached)
    {
        if (!s_Loaded) Load();

        LeaderboardEntry e;
        e.name = name.empty() ? "Player" : name;
        e.score = score;
        e.level = levelName;
        e.wave = waveReached;

        s_Entries.push_back(e);

        // Sort highest first
        std::sort(s_Entries.begin(), s_Entries.end(),
            [](const LeaderboardEntry& a, const LeaderboardEntry& b)
            { return a.score > b.score; });

        // Keep only top MAX_ENTRIES.
        // static_cast applied to safely compare sizes.
        if (static_cast<int>(s_Entries.size()) > MAX_ENTRIES)
            s_Entries.resize(MAX_ENTRIES);

        std::cout << "[Leaderboard] New entry: " << e.name
            << " | " << e.score << " pts"
            << " | " << e.level
            << " | Wave " << e.wave << "\n";

        Save();
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