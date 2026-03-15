#pragma once
#include <string>
#include "AEEngine.h"


struct AudioManager {
	static constexpr int AUDIO_COUNT = 4;

	static void Init();
	static void Free();

	static void PlaySFX(const std::string& name);
	static void	PlayMusic(const std::string& name);
	static void StopMusic(const std::string& name);

	static void SetMasterVolume(f32 volume);

	//static std::array<std::pair<std::string, std::string> AudioName;
	static std::string audioNames[AUDIO_COUNT];
	static AEAudio audioResource[AUDIO_COUNT];
	static AEAudioGroup group[AUDIO_COUNT];
	static float masterVolume;
};