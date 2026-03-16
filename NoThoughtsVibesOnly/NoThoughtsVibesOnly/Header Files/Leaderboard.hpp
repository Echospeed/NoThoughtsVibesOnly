#pragma once
// ============================================================================
// Leaderboard.hpp - Top-10 Score Tracking (RapidJSON-backed)
// ============================================================================
// Saves and loads the top 10 scores to/from "Assets/leaderboard.json".
// Scores are stored with a player name, score value, level name, and wave
// reached. The list is always kept sorted highest-first, max 10 entries.
//
// USAGE:
// ----------------------------------------------------------------------------
//   // On win - submit a score:
//   Leaderboard::Submit("Player", score, "Level 1", wavesCleared);
//
//   // To display the board (e.g. in WinPage):
//   const auto& entries = Leaderboard::GetEntries();
//   for (auto& e : entries)
//       std::cout << e.name << " : " << e.score << "\n";
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

// Defines the data structure for a single leaderboard entry
struct LeaderboardEntry
{
    std::string name = "Player";   // Player name
    int         score = 0;         // Final score
    std::string level = "Unknown"; // Level name e.g. "Level 1"
    int         wave = 0;          // Wave reached
};

namespace Leaderboard
{
    static const char* SAVE_PATH = "Assets/leaderboard.json";
    static const int   MAX_ENTRIES = 20;

    // Load scores from SAVE_PATH. Safe to call even if file doesn't exist.
    void Load();

    // Write current scores to SAVE_PATH.
    void Save();

    // Add a new score, re-sort, trim to MAX_ENTRIES, and save.
    void Submit(const std::string& name, int score,
        const std::string& levelName, int waveReached);

    // Returns the current sorted leaderboard (highest score first).
    const std::vector<LeaderboardEntry>& GetEntries();

    // Wipe the leaderboard and save the empty file.
    void Clear();
}