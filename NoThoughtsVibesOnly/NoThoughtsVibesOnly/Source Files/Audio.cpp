#include "pch.hpp"
#include "Audio.hpp"

Audio::Audio(const std::string& filepath, s8 loop, f32 volume, f32 pitch, AudioType type)
	: isLoaded(false), loop(loop), volume(volume), pitch(pitch), audioType(type), audioResource({nullptr}), group({ nullptr })
{
	if (audioType == AudioType::SOUND)
	{
		audioResource = AEAudioLoadSound(filepath.c_str());

		AEAudioGroup sound_effect = AEAudioCreateGroup();
		this->group = sound_effect;

		isLoaded = true;
	}
	else // MUSIC
	{
		audioResource = AEAudioLoadMusic(filepath.c_str());
		isLoaded = true;
	}
}

Audio::~Audio()
{
	Free();
}

void Audio::Play()
{
	if (!isLoaded) return;
	AEAudioPlay(this->audioResource, this->group, this->volume, this->pitch, this->loop);
}

void Audio::SetVolume(f32 volume, f32 pitch, s8 loop)
{
	this->volume = volume;
	this->pitch = pitch;
	this->loop = loop;

	if (isLoaded) // Only set if group is valid
	{
		AEAudioSetGroupVolume(this->group, this->volume);
		AEAudioSetGroupPitch(this->group, this->pitch);
	}
}

void Audio::Pause()
{
	if (isLoaded)
	{
		AEAudioPauseGroup(this->group); // Pauses the group 
	}
}

void Audio::Resume()
{
	if(isLoaded)
	{
		AEAudioResumeGroup(this->group); // Resume the group
	}
}

void Audio::Stop()
{
	if (isLoaded)
	{
		AEAudioStopGroup(this->group);
	}
}

void Audio::Free()
{
	if (isLoaded)
	{
		AEAudioUnloadAudio(this->audioResource);
		AEAudioUnloadAudioGroup(this->group);
		isLoaded = false;
	}
}