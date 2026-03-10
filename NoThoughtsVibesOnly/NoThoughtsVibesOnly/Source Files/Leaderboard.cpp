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

#pragma warning(push)
#pragma warning(disable: 26495) // uninitialized member
#pragma warning(disable: 26494) // variable not initialized
#pragma warning(disable: 26482) // enum used as index
#pragma warning(disable: 26451) // arithmetic overflow checks
#include "include/rapidjson/document.h"
#include "include/rapidjson/filereadstream.h"
#include "include/rapidjson/filewritestream.h"
#include "include/rapidjson/prettywriter.h"
#pragma warning(pop)

#include <algorithm>
#include <iostream>
#include <cstdio>

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

        FILE* fp = fopen(SAVE_PATH, "rb");
        if (!fp)
        {
            std::cout << "[Leaderboard] No save file found - starting fresh.\n";
            s_Loaded = true;
            return;
        }

        char buf[8192]{};
        rapidjson::FileReadStream is(fp, buf, sizeof(buf));

        rapidjson::Document doc;
        doc.ParseStream(is);
        fclose(fp);

        if (doc.HasParseError() || !doc.HasMember("entries") || !doc["entries"].IsArray())
        {
            std::cout << "[Leaderboard] WARNING: Corrupt save file - starting fresh.\n";
            s_Loaded = true;
            return;
        }

        const auto& arr = doc["entries"].GetArray();
        for (rapidjson::SizeType i = 0; i < arr.Size() && (int)i < MAX_ENTRIES; ++i)
        {
            const auto& obj = arr[i];
            LeaderboardEntry e;

            e.name = obj.HasMember("name") && obj["name"].IsString() ? obj["name"].GetString() : "Player";
            e.score = obj.HasMember("score") && obj["score"].IsInt() ? obj["score"].GetInt() : 0;
            e.level = obj.HasMember("level") && obj["level"].IsString() ? obj["level"].GetString() : "Unknown";
            e.wave = obj.HasMember("wave") && obj["wave"].IsInt() ? obj["wave"].GetInt() : 0;

            s_Entries.push_back(e);
        }

        // Ensure sorted order after load
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
        FILE* fp = fopen(SAVE_PATH, "wb");
        if (!fp)
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
            // so we copy strings using the allocator
            rapidjson::Value name(e.name.c_str(), (rapidjson::SizeType)e.name.size(), alloc);
            rapidjson::Value level(e.level.c_str(), (rapidjson::SizeType)e.level.size(), alloc);

            obj.AddMember("name", name, alloc);
            obj.AddMember("score", e.score, alloc);
            obj.AddMember("level", level, alloc);
            obj.AddMember("wave", e.wave, alloc);

            arr.PushBack(obj, alloc);
        }

        doc.AddMember("entries", arr, alloc);

        // Write with pretty formatting so the file is human-readable
        char buf[8192]{};
        rapidjson::FileWriteStream os(fp, buf, sizeof(buf));
        rapidjson::PrettyWriter<rapidjson::FileWriteStream> writer(os);
        doc.Accept(writer);

        fclose(fp);
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

        // Keep only top MAX_ENTRIES
        if ((int)s_Entries.size() > MAX_ENTRIES)
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