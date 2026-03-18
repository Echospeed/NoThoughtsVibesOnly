#include "pch.hpp"
#include "AudioManager.hpp"
#include <iostream>

std::unordered_map<std::string, AudioManager::AudioBundle> AudioManager::s_AudioMap;
std::string AudioManager::s_CurrentMusicName = "";
float AudioManager::s_MasterVolume = 1.0f;

void AudioManager::Init()
{
    LoadAudio("MenuMusic", "Assets/Audio/MainMenusfx.wav", true);
    LoadAudio("GameMusic", "Assets/Audio/GameBGsfx.wav", true);
    LoadAudio("Shoot", "Assets/Audio/ShootingSound.wav", false);
    LoadAudio("LevelUp", "Assets/Audio/Powerupsfx.wav", false);

    //std::cout << "[AudioManager] Native AEAudio assets and groups loaded.\n";
}

void AudioManager::LoadAudio(const std::string& name, const char* filepath, bool isMusic)
{
    if (s_AudioMap.find(name) != s_AudioMap.end()) return;

    AEAudio audio;
    if (isMusic) audio = AEAudioLoadMusic(filepath);
    else         audio = AEAudioLoadSound(filepath);

    if (AEAudioIsValidAudio(audio))
    {
        AEAudioGroup group = AEAudioCreateGroup();
        s_AudioMap[name] = { audio, group };
    }
    else
    {
        //std::cout << "[AudioManager] ERROR: Alpha Engine failed to load: " << filepath << "\n";
    }
}

void AudioManager::PlaySFX(const std::string& name)
{
    auto it = s_AudioMap.find(name);
    if (it != s_AudioMap.end())
    {
        AEAudioPlay(it->second.audio, it->second.group, s_MasterVolume, 1.0f, 0);
    }
    else
    {
        //std::cout << "[AudioManager] WARNING: SFX '" << name << "' not loaded!\n";
    }
}

void AudioManager::PlayMusic(const std::string& name)
{
    auto it = s_AudioMap.find(name);
    if (it != s_AudioMap.end())
    {
        StopMusic();

        s_CurrentMusicName = name;
        AEAudioPlay(it->second.audio, it->second.group, s_MasterVolume * 0.5f, 1.0f, -1);
    }
    else
    {
        //std::cout << "[AudioManager] WARNING: Music '" << name << "' not loaded!\n";
    }
}

void AudioManager::PauseMusic(const std::string& name)
{
    const std::string targetName = name.empty() ? s_CurrentMusicName : name;
    if (targetName.empty()) return;

    auto it = s_AudioMap.find(targetName);
    if (it != s_AudioMap.end() && AEAudioIsValidGroup(it->second.group))
    {
        AEAudioPauseGroup(it->second.group);
    }
}

void AudioManager::ResumeMusic(const std::string& name)
{
    const std::string targetName = name.empty() ? s_CurrentMusicName : name;
    if (targetName.empty()) return;

    auto it = s_AudioMap.find(targetName);
    if (it != s_AudioMap.end() && AEAudioIsValidGroup(it->second.group))
    {
        AEAudioResumeGroup(it->second.group);
    }
}

void AudioManager::StopMusic(const std::string& name)
{
    std::string targetName = name.empty() ? s_CurrentMusicName : name;
    if (targetName.empty()) return;

    auto it = s_AudioMap.find(targetName);
    if (it != s_AudioMap.end())
    {
        if (AEAudioIsValidGroup(it->second.group))
        {
            AEAudioStopGroup(it->second.group);
        }
    }
    else
    {
        //std::cout << "[AudioManager] WARNING: Cannot stop '" << targetName << "', not found!\n";
    }

    if (s_CurrentMusicName == targetName)
    {
        s_CurrentMusicName = "";
    }
}

void AudioManager::Free()
{
    StopMusic();

    for (auto& pair : s_AudioMap)
    {
        if (AEAudioIsValidAudio(pair.second.audio)) {
            AEAudioUnloadAudio(pair.second.audio);
        }
        if (AEAudioIsValidGroup(pair.second.group)) {
            AEAudioUnloadAudioGroup(pair.second.group);
        }
    }
    s_AudioMap.clear();
    //std::cout << "[AudioManager] Native AEAudio resources and groups freed.\n";
}

void AudioManager::SetMasterVolume(float volume)
{
    s_MasterVolume = volume;

    if (!s_CurrentMusicName.empty())
    {
        auto it = s_AudioMap.find(s_CurrentMusicName);
        if (it != s_AudioMap.end() && AEAudioIsValidGroup(it->second.group))
        {
            AEAudioSetGroupVolume(it->second.group, s_MasterVolume * 0.5f);
        }
    }
}