# Pew Pew Galaxy

A top-down, bullet-hell space shooter written in C++ for Windows. Fly a ship around a bounded arena, survive escalating waves of enemies, level up mid-run by picking one of four power-ups, and fight a boss at the end of each level. Built by a team of four for CSD1451 Software Engineering 2 at DigiPen Singapore (Jan - Apr 2026).

Repository name: `NoThoughtsVibesOnly`.

## Features

- **Wave-based combat.** Level 1 has 5 waves with no boss. Level 2 has 10 waves with a boss on the final wave. Endless mode runs forever with a boss every 5 rounds.
- **Three enemy types.** Walkers, melee chasers and ranged enemies, each scaling with the round number.
- **In-run progression.** Killing enemies earns experience. Each level-up offers four power-up choices drawn from upgrades such as speed, bullet damage, bullet count, area damage and lifesteal.
- **Minimap, particles, audio and animated sprites.**
- **Top-10 leaderboard** saved to a JSON file, with a name filter that blocks words listed in `Assets/blocklist.txt`.
- **Data-driven design.** Enemy waves, power-ups, player stats, levels and gameplay settings all live in JSON files under `Assets/`, so the game can be rebalanced without recompiling. See `Assets/PewPewGalaxy_JSON_Config_Guide.docx` for the format.

## Controls

| Input | Action |
|---|---|
| W / A / S / D | Move |
| Left Arrow / Right Arrow | Rotate |
| Hold left mouse button | Shoot toward the cursor (10 shots per second) |
| Right mouse click | Shotgun blast in a cone toward the cursor (0.3 second cooldown) |
| 1 / 2 / 3 / 4 | Pick a power-up when you level up |
| E | Activate invincibility |
| Left Shift | Show or hide the stats panel |
| Esc | Pause and resume |
| Tab (on the pause screen) | Restart |

The game also has a debug mode toggled with Left Ctrl. It is only for development, so it is not listed above.

## Build and run

You need Windows 10 or 11 and Visual Studio 2022 with the "Desktop development with C++" workload (the project uses the v143 toolset and C++14).

1. Clone the repository.
2. Open `NoThoughtsVibesOnly/NoThoughtsVibesOnly.sln` in Visual Studio.
3. Select the **x64** platform and the **Debug** or **Release** configuration.
4. Press **F5** to build and run.

The DigiPen Alpha Engine headers and libraries are included under `NoThoughtsVibesOnly/Extern/AlphaEngine`, so no extra setup is needed.

## How the code is organised

```text
NoThoughtsVibesOnly/
├── Assets/                      Textures, audio, fonts and the JSON config files
├── Extern/AlphaEngine/          DigiPen's 2D graphics and input library
└── NoThoughtsVibesOnly/
    ├── Header Files/            .hpp files
    └── Source Files/            .cpp files
```

- **`GameObject`** is the base class for every entity (`Player`, `NPC`, `Bullet`, `Button`). Each object registers itself in the right list when it is constructed and releases its texture when it is destroyed.
- **`StateManager`** switches between the game's pages: splash, main menu, level select, gameplay, pause, win and finish.
- **`WaveSystem`** schedules and spawns enemies for each round. **`PowerUpSystem`** handles experience, level-ups and the four power-up choices.
- **`GameConfig`** and **`LevelConfig`** load the JSON files with RapidJSON.
- **Object pooling.** Bullets and particles are allocated once up front and reused, which avoids allocating memory in the middle of a frame.

## Team

Chia Wenjie, John Chiow, Liu Yan Bin and Stanley Lu.

## Credits

Built on the DigiPen Alpha Engine. Uses [RapidJSON](https://rapidjson.org/). The sprites, fonts and audio come from free, open-source sources.
