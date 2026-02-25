// ============================================================================
// Audio.cpp - Audio Playback Wrapper Implementation
// ============================================================================
// See Audio.hpp for full documentation and usage examples.
// ============================================================================

#include "pch.hpp"
#include "Audio.hpp"
#include <iostream>

// ============================================================================
// Constructor
// ============================================================================
// Loads the audio file and creates an AEAudioGroup for playback control.
// If the filepath is empty or loading fails, isLoaded remains false and all
// subsequent calls (Play, Pause, etc.) will safely no-op.
// ============================================================================
Audio::Audio(const std::string& filepath, s8 loop, f32 volume, f32 pitch, AudioType type)
    : isLoaded(false)
    , loop(loop)
    , volume(volume)
    , pitch(pitch)
    , audioType(type)
    , audioResource({ nullptr })
    , group({ nullptr })
{
    if (filepath.empty())
    {
        std::cout << "[Audio] ERROR: Empty filepath provided.\n";
        return;
    }

    if (audioType == AudioType::SOUND)
    {
        audioResource = AEAudioLoadSound(filepath.c_str());
        group = AEAudioCreateGroup();
    }
    else // AudioType::MUSIC
    {
        audioResource = AEAudioLoadMusic(filepath.c_str());
        group = AEAudioCreateGroup();
    }

    isLoaded = true;
}

// ============================================================================
// Destructor
// ============================================================================
Audio::~Audio()
{
    // Free() is intentionally NOT called here because AEAudioUnloadAudio can
    // crash if called after AESysExit(). Callers should Stop() before deletion.
    // If you need guaranteed cleanup, call audio->Stop() before deleting.
}

// ============================================================================
// Play
// ============================================================================
// Plays the audio resource through the group using the stored parameters.
// Safe to call multiple times - SFX will overlap (if not in a limited group).
// ============================================================================
void Audio::Play()
{
    if (!isLoaded) return;
    AEAudioPlay(audioResource, group, volume, pitch, loop);
}

// ============================================================================
// SetVolume
// ============================================================================
// Updates stored parameters and immediately applies them to the audio group.
// Call this to adjust music volume dynamically (e.g. fade-out on death).
// ============================================================================
void Audio::SetVolume(f32 newVolume, f32 newPitch, s8 newLoop)
{
    volume = newVolume;
    pitch = newPitch;
    loop = newLoop;

    if (isLoaded)
    {
        AEAudioSetGroupVolume(group, volume);
        AEAudioSetGroupPitch(group, pitch);
    }
}

// ============================================================================
// Pause
// ============================================================================
// Pauses all audio in the group. Used when the game enters the pause state.
// Call Resume() to continue from where it left off.
// ============================================================================
void Audio::Pause()
{
    if (isLoaded) AEAudioPauseGroup(group);
}

// ============================================================================
// Resume
// ============================================================================
// Resumes all audio in the group after a Pause().
// ============================================================================
void Audio::Resume()
{
    if (isLoaded) AEAudioResumeGroup(group);
}

// ============================================================================
// Stop
// ============================================================================
// Immediately stops all audio in the group.
// Use before state transitions to prevent audio bleeding between states.
// ============================================================================
void Audio::Stop()
{
    if (isLoaded) AEAudioStopGroup(group);
}

// ============================================================================
// Free (private)
// ============================================================================
// Unloads the audio resource and destroys the group.
// Only safe to call before AESysExit().
// ============================================================================
void Audio::Free()
{
    if (isLoaded)
    {
        AEAudioUnloadAudio(audioResource);
        AEAudioUnloadAudioGroup(group);
        isLoaded = false;
    }
}