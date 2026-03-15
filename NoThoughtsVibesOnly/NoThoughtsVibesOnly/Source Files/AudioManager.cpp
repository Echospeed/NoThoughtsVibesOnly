#include "pch.hpp"
#include "AudioManager.hpp"
#include "AEAudio.h"
#include "AETypes.h"
#include <iostream>

std::string AudioManager::audioNames[AUDIO_COUNT]{ "MenuMusic" , "test1" , "test2", "test3"};
AEAudio AudioManager::audioResource[AUDIO_COUNT];
AEAudioGroup AudioManager::group[AUDIO_COUNT];
float AudioManager::masterVolume = 1.0f;

void AudioManager::Init()
{
	audioResource[0] = AEAudioLoadMusic("Assets/Audio/BATTLE-MILITARY_GEN-HDF-03135.wav");
	group[0] = AEAudioCreateGroup();

	audioResource[1] = AEAudioLoadSound("Assets/Audio/BATTLE-MILITARY_GEN-HDF-03135.wav");
	group[1] = AEAudioCreateGroup();

	audioResource[2] = AEAudioLoadSound("Assets/Audio/SCI-FI-LASER_GEN-HDF-20715.wav");
	group[2] = AEAudioCreateGroup();

	audioResource[3] = AEAudioLoadSound("Assets/Audio/SCI-FI-LASER_GEN-HDF-20715.wav");
	group[3] = AEAudioCreateGroup();
}

void AudioManager::Free()
{
	for (int i = 0; i < AUDIO_COUNT; ++i)
	{
		AEAudioUnloadAudio(audioResource[i]);
		AEAudioUnloadAudioGroup(group[i]);
	}
}

void AudioManager::PlaySFX(const std::string& name)
{
	for (int i = 0; i < AUDIO_COUNT; ++i)
	{
		if (audioNames[i] == name)
		{
			AEAudioPlay(audioResource[i], group[i], masterVolume, 1.0f, 0);
			return;
		}
	}
	std::cout << "[AudioManager] ERROR: SFX '" << name << "' not found.\n";
}

void AudioManager::PlayMusic(const std::string& name)
{
	for (int i = 0; i < AUDIO_COUNT; ++i)
	{
		if (audioNames[i] == name)
		{
			AEAudioPlay(audioResource[i], group[i], masterVolume, 1.0f, -1);
			return;
		}
	}
	std::cout << "[AudioManager] ERROR: Music '" << name << "' not found.\n";
}

void AudioManager::StopMusic(const std::string& name)
{
	for (int i = 0; i < AUDIO_COUNT; ++i)
	{
		if (audioNames[i] == name)
		{
			AEAudioStopGroup(group[i]);
			return;
		}
	}
	std::cout << "[AudioManager] ERROR: Music '" << name << "' not found.\n";
}

void AudioManager::SetMasterVolume(f32 volume)
{
	masterVolume = (volume < 0.0f) ? 0.0f : (volume > 1.0f) ? 1.0f : volume;
	for (int i = 0; i < AUDIO_COUNT; ++i)
	{
		AEAudioSetGroupVolume(group[i], masterVolume);
	}
}