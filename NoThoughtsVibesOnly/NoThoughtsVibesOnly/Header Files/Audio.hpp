#pragma once
// ============================================================================
// Audio.hpp - Audio Playback Wrapper
// ============================================================================
// Wraps Alpha Engine's audio API into a clean, safe class.
// Supports two audio types: SOUND (one-shot SFX) and MUSIC (looping BGM).
//
// USAGE EXAMPLE:
// ----------------------------------------------------------------------------
//   // In Game_Load():
//   Audio* bgMusic  = new Audio("Assets/music.mp3",  -1,  0.5f, 1.0f, AudioType::MUSIC);
//   Audio* shootSFX = new Audio("Assets/shoot.mp3",   0,  1.0f, 1.0f, AudioType::SOUND);
//
//   // In Game_Init() or whenever needed:
//   bgMusic->Play();          // Start looping music
//   shootSFX->Play();         // Fire a one-shot sound
//
//   // While paused:
//   bgMusic->Pause();
//   bgMusic->Resume();
//
//   // On state exit:
//   bgMusic->Stop();
//
//   // In Game_Unload():
//   delete bgMusic;
//   delete shootSFX;
// ============================================================================

#include "AEEngine.h"
#include <string>

// ----------------------------------------------------------------------------
// AudioType - Determines how the audio resource is loaded and managed.
//   SOUND : Loaded as a one-shot sound effect via AEAudioLoadSound().
//   MUSIC : Loaded as a streaming music file via AEAudioLoadMusic().
// ----------------------------------------------------------------------------
enum class AudioType
{
    SOUND,
    MUSIC
};

// ============================================================================
// class Audio
// ============================================================================
// Manages a single audio resource (sound effect or background music).
// Each instance owns one AEAudio resource and one AEAudioGroup.
//
// Copying is disabled to prevent double-free crashes - always use pointers.
// ============================================================================
class Audio
{
public:
    // ------------------------------------------------------------------------
    // Constructor
    // filepath : Path to the audio file (e.g. "Assets/shoot.mp3")
    // loop     : Number of times to loop. -1 = infinite, 0 = play once.
    // volume   : Playback volume [0.0 - 1.0]
    // pitch    : Playback pitch multiplier (1.0 = normal)
    // type     : AudioType::SOUND or AudioType::MUSIC
    // ------------------------------------------------------------------------
    Audio(const std::string& filepath, s8 loop, f32 volume, f32 pitch, AudioType type);

    // Destructor - calls Free() automatically
    ~Audio();

    // Disable copy semantics to prevent double-free on the audio resource
    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

    // ------------------------------------------------------------------------
    // Play - Plays the audio using the stored volume, pitch, and loop settings.
    // Safe to call even if loading failed (guarded by isLoaded).
    // ------------------------------------------------------------------------
    void Play();

    // ------------------------------------------------------------------------
    // SetVolume - Updates the volume, pitch, and loop count at runtime.
    // Also immediately applies the new volume/pitch to the audio group.
    // ------------------------------------------------------------------------
    void SetVolume(f32 volume, f32 pitch, s8 loop);

    // ------------------------------------------------------------------------
    // Pause / Resume / Stop - Group-level playback control.
    // Pause() and Resume() are useful for pausing the game without unloading.
    // Stop() halts all playback in the group (use before state transitions).
    // ------------------------------------------------------------------------
    void Pause();
    void Resume();
    void Stop();

private:
    bool         isLoaded{ false };   // Guards all playback calls against unloaded state
    s8           loop{ -1 };          // Loop count: -1 = infinite, 0 = once
    f32          volume{ 1.0f };      // Playback volume [0.0 - 1.0]
    f32          pitch{ 1.0f };       // Pitch multiplier (1.0 = unchanged)
    AEAudio      audioResource{};   // Handle to the loaded AEAudio resource
    AudioType    audioType;         // Determines which AE loader to use
    AEAudioGroup group{};           // AE audio group used for pause/resume/stop

    // ------------------------------------------------------------------------
    // Free - Unloads the audio resource and group. Called by destructor.
    // ------------------------------------------------------------------------
    void Free();
};