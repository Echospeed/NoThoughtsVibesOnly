#pragma once
// Author: Liu Yan Bin
// Co-Author: John Chiow

// ============================================================================
// Leaderboard.hpp - Top-10 Score Tracking (RapidJSON-backed)
// ============================================================================
// Saves and loads the top 10 scores to/from "Assets/leaderboard.json".
// Scores are stored with a player name, score value, level name, and wave
// reached. The list is always kept sorted highest-first, max 10 entries.
//
// NAME FILTERING:
// ----------------------------------------------------------------------------
//   Submit() now returns bool.
//     true  = name was clean, score was saved.
//     false = name was rejected (contained a blocked word).
//   Blocked words are loaded once from Assets/blocklist.txt (one word per
//   line). The check is case-insensitive and matches substrings, so a word
//   hidden inside a longer name is still caught.
//
// USAGE:
// ----------------------------------------------------------------------------
//   bool ok = Leaderboard::Submit("Player", score, "Level 1", wavesCleared);
//   if (!ok) { /* show "INVALID NAME" error to player */ }
//
// FILE FORMAT (Assets/leaderboard.json):
// ----------------------------------------------------------------------------
//   {
//     "entries": [
//       { "name": "Player", "score": 1000, "level": "Level 1", "wave": 5 },
//       ...
//     ]
//   }
// ============================================================================
#include "pch.hpp"
#include <string>
#include <vector>

struct LeaderboardEntry
{
    std::string name = "Player";
    int         score = 0;
    std::string level = "Unknown";
    int         wave = 0;
};

namespace Leaderboard
{
    static const char* SAVE_PATH = "Assets/leaderboard.json";
    static const char* BLOCKLIST_PATH = "Assets/blocklist.txt";
    static const int   MAX_ENTRIES = 20;

    // Load scores from SAVE_PATH. Safe to call even if file doesn't exist.
    void Load();

    // Write current scores to SAVE_PATH.
    void Save();

    // Add a new score, re-sort, trim to MAX_ENTRIES, and save.
    // Returns true  if the name passed the filter and the score was saved.
    // Returns false if the name was rejected (contained a blocked word).
    bool Submit(const std::string& name, int score,
        const std::string& levelName, int waveReached);

    // Returns the current sorted leaderboard (highest score first).
    const std::vector<LeaderboardEntry>& GetEntries();

    // Wipe the leaderboard and save the empty file.
    void Clear();

    // Returns true if the name contains a blocked word.
    // Public so FinishPage can pre-validate before calling Submit if needed.
    bool IsNameBlocked(const std::string& name);
}