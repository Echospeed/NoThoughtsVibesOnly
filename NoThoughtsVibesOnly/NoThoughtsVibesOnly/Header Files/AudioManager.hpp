#pragma once

#include <string>
#include <unordered_map>
#include "AEEngine.h"

class AudioManager
{
public:
    static void Init();
    static void Free();

    static void LoadAudio(const std::string& name, const char* filepath, bool isMusic = false);

    static void PlaySFX(const std::string& name);
    static void PlayMusic(const std::string& name);

    // Can be called as PauseMusic() / ResumeMusic() (current track) or with a name.
    static void PauseMusic(const std::string& name = "");
    static void ResumeMusic(const std::string& name = "");

    // Default argument allows it to be called as StopMusic() OR StopMusic("Name")
    static void StopMusic(const std::string& name = "");

    static void SetMasterVolume(float volume);

private:
    struct AudioBundle {
        AEAudio audio;
        AEAudioGroup group;
    };

    static std::unordered_map<std::string, AudioBundle> s_AudioMap;

    static std::string s_CurrentMusicName;
    static float s_MasterVolume;
};