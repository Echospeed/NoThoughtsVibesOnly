#pragma once
#include "AEEngine.h" // Assuming this is the Alpha Engine header
#include <string>

// Enum to handle the two types of Alpha Engine audio
enum class AudioType
{
    SOUND,
    MUSIC
};

class Audio
{
public:
    // Constructor: Takes the filepath and the type (Sound vs Music)
    Audio(const std::string& filepath, s8 loop, f32 volume, f32 pitch, AudioType type);

    // Destructor: Automatically cleans up the memory
    ~Audio();

    // Disable copying to prevent double-free memory crashes
    Audio(const Audio&) = delete;
    Audio& operator=(const Audio&) = delete;

    // Play function requires knowing which group to play on, plus volume and pitch
    void Play();

	// Set volume for a specific group (useful for music vs sound effects)
	void SetVolume(f32 volume, f32 pitch, s8 loop);

	// Pause function for music or sound effects (pauses the entire group to which the audio belongs)
	void Pause();

	// Resume function for music or sound effects (resumes the entire group to which the audio belongs)
    void Resume();

	// Set stopping audio for a specific group
	void Stop();

	// Lower volume of the music group
	void LowerMusicVolume(f32 volume);

	// Increase volume of the music group
	void IncreaseMusicVolume(f32 volume);

private:
    bool isLoaded{false};       // Safety check
    s8 loop{-1};                // Store loop count for potential future use
    f32 volume{1};              // Store volume for potential future use
    f32 pitch{1};               // Store pitch for potential future use
    AEAudio audioResource;      // The actual loaded audio
    AudioType audioType;        // Keeps track of what type it is
	AEAudioGroup group;         // Store group for potential future use
    void Free();
};