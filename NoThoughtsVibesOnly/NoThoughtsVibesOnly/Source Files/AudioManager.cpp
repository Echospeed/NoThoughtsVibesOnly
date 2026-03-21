#include "pch.hpp"
#include "AudioManager.hpp"
#include <iostream>

// We have to define our static variables here in the .cpp file.
// If we leave them in the header, the compiler gets mad and says they don't exist in memory.
std::unordered_map<std::string, AudioManager::AudioBundle> AudioManager::s_AudioMap;

// Start with an empty string because no music is playing when the game first boots up.
std::string AudioManager::s_CurrentMusicName = "";
float AudioManager::s_MasterVolume = 1.0f;

// ============================================================================
// Init (Booting up the sounds)
// ============================================================================
void AudioManager::Init()
{
    // Loading all our files right when the game starts.
    // 'true' means it's a long song, so it streams from the hard drive.
    LoadAudio("MenuMusic", "Assets/Audio/MainMenusfx.wav", true);
    LoadAudio("GameMusic", "Assets/Audio/GameBGsfx.wav", true);

    // 'false' means it's a short sound effect, so we cram the whole thing into RAM 
    // so it can play instantly when we shoot.
    LoadAudio("Shoot", "Assets/Audio/ShootingSound.wav", false);
    LoadAudio("LevelUp", "Assets/Audio/Powerupsfx.wav", false);

    std::cout << "[AudioManager] All sounds are loaded into the dictionary!\n";
}

// ============================================================================
// LoadAudio (Putting the sound into our dictionary)
// ============================================================================
void AudioManager::LoadAudio(const std::string& name, const char* filepath, bool isMusic)
{
    // Check if we already loaded a sound with this name.
    // .find() searches the dictionary. If it does NOT equal .end(), it found something!
    // Return early so we don't accidentally load the same file twice and waste memory.
    if (s_AudioMap.find(name) != s_AudioMap.end()) return;

    // Create an empty box to hold the Alpha Engine audio data
    AEAudio audio;

    // Check our boolean flag to see which Alpha Engine function we should use
    if (isMusic) {
        audio = AEAudioLoadMusic(filepath);
    }
    else {
        audio = AEAudioLoadSound(filepath);
    }

    // Alpha Engine uses C-style structs, so we can't just say "if (audio)".
    // We have to use their special function to check if the file actually loaded successfully.
    if (AEAudioIsValidAudio(audio))
    {
        // Every sound needs its own "Group" (like a channel) to play on.
        AEAudioGroup group = AEAudioCreateGroup();

        // Bundle the sound and its channel together, and lock them in our dictionary
        // using the string 'name' as the key.
        s_AudioMap[name] = { audio, group };
    }
    else
    {
        // If we typed the file path wrong, this warns us in the console.
        std::cout << "[AudioManager] Oops, failed to load: " << filepath << "\n";
    }
}

// ============================================================================
// PlaySFX (Playing short sound effects)
// ============================================================================
void AudioManager::PlaySFX(const std::string& name)
{
    // 'auto' is a shortcut so I don't have to type out the massive iterator type.
    // 'it' points to the result of our dictionary search.
    auto it = s_AudioMap.find(name);

    // Did we actually find the sound in the dictionary?
    if (it != s_AudioMap.end())
    {
        // it->second grabs the 'Value' from our map (which is our AudioBundle struct).
        // Then we grab the specific audio and group from inside that bundle.
        // The '0' at the end means play it exactly once.
        AEAudioPlay(it->second.audio, it->second.group, s_MasterVolume, 1.0f, 0);
    }
    else
    {
        std::cout << "[AudioManager] Can't play '" << name << "', I don't have it!\n";
    }
}

// ============================================================================
// PlayMusic (Playing looping background tracks)
// ============================================================================
void AudioManager::PlayMusic(const std::string& name)
{
    auto it = s_AudioMap.find(name);
    if (it != s_AudioMap.end())
    {
        // Stop whatever song is currently playing first so they don't overlap.
        StopMusic();

        // Remember the name of the new song we are about to play.
        s_CurrentMusicName = name;

        // The '-1' at the end tells Alpha Engine to loop this forever.
        // Multiply volume by 0.5 so the music doesn't drown out the laser sounds.
        AEAudioPlay(it->second.audio, it->second.group, s_MasterVolume * 0.5f, 1.0f, -1);
    }
    else
    {
        std::cout << "[AudioManager] Can't play music '" << name << "', I don't have it!\n";
    }
}

// ============================================================================
// StopMusic
// ============================================================================
void AudioManager::StopMusic(const std::string& name)
{
    // If we didn't type a name in the function call, just use the one we are currently tracking.
    std::string targetName = name.empty() ? s_CurrentMusicName : name;

    // If the target is STILL empty, it means no music is playing at all.
    // Return early here so we don't waste time searching the dictionary for a blank string.
    if (targetName.empty()) return;

    // Search the dictionary for the track we want to stop.
    auto it = s_AudioMap.find(targetName);
    if (it != s_AudioMap.end())
    {
        // Check if the group is valid, then stop it.
        if (AEAudioIsValidGroup(it->second.group))
        {
            AEAudioStopGroup(it->second.group);
        }
    }

    // If we just stopped the song that we were tracking globally, clear out the tracker
    // so the game knows it's quiet now.
    if (s_CurrentMusicName == targetName)
    {
        s_CurrentMusicName = "";
    }
}

// ============================================================================
// PauseMusic (Freezing a song)
// ============================================================================
void AudioManager::PauseMusic(const std::string& name)
{
    // Use the typed name, or default to the currently playing song
    std::string targetName = name.empty() ? s_CurrentMusicName : name;

    // Return early if there is no target
    if (targetName.empty()) return;

    auto it = s_AudioMap.find(targetName);

    if (it != s_AudioMap.end() && AEAudioIsValidGroup(it->second.group))
    {
        AEAudioPauseGroup(it->second.group);
    }
}

// ============================================================================
// ResumeMusic (Unfreezing a song)
// ============================================================================
void AudioManager::ResumeMusic(const std::string& name)
{
    // Use the typed name, or default to the currently playing song
    std::string targetName = name.empty() ? s_CurrentMusicName : name;

    // Return early if there is no target
    if (targetName.empty()) return;

    auto it = s_AudioMap.find(targetName);

    if (it != s_AudioMap.end() && AEAudioIsValidGroup(it->second.group))
    {
        AEAudioResumeGroup(it->second.group);
    }
}

// ============================================================================
// SetMasterVolume
// ============================================================================
void AudioManager::SetMasterVolume(float volume)
{
    s_MasterVolume = volume;

    // Check if a song is actually playing right now.
    if (!s_CurrentMusicName.empty())
    {
        // Find the currently playing song in the dictionary.
        auto it = s_AudioMap.find(s_CurrentMusicName);

        // If we found it, and its group is valid, change its volume in real-time.
        if (it != s_AudioMap.end() && AEAudioIsValidGroup(it->second.group))
        {
            AEAudioSetGroupVolume(it->second.group, s_MasterVolume * 0.5f);
        }
    }
}

// ============================================================================
// Free (Cleaning up our mess)
// ============================================================================
void AudioManager::Free()
{
    // Silence everything first.
    StopMusic();

    // Loop through every single item stored in our dictionary.
    for (auto& pair : s_AudioMap)
    {
        // Have to ask Alpha Engine if the audio is valid before we delete it,
        // otherwise the game might crash trying to delete a null pointer.
        if (AEAudioIsValidAudio(pair.second.audio)) {
            AEAudioUnloadAudio(pair.second.audio);
        }
        if (AEAudioIsValidGroup(pair.second.group)) {
            AEAudioUnloadAudioGroup(pair.second.group);
        }
    }

    // Empty the dictionary completely.
    s_AudioMap.clear();
    std::cout << "[AudioManager] All audio memory has been cleaned up.\n";
}