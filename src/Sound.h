#pragma once
#include "SDL3\SDL_audio.h"

struct SDL_AudioStream;

class Sound
{
public:
	Sound();

	bool Init();
	void Shutdown();
	
	static void AudioCallback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount);
	void StartBeep() { isPlaying = true; }
	void StopBeep() { isPlaying = false; }
	bool IsPlaying() const { return isPlaying;  }

private:
	static constexpr float BEEP_AMPLITUDE = 0.2f;
	static constexpr float BEEP_FREQUENCY = 880.f;
	static constexpr float ATTACK_DECAY_DURATION_MS = 0.f;
	static const int FREQUENCY = 44100;

	SDL_AudioDeviceID deviceID;
	float samples[FREQUENCY] = {};
	float v;
	bool isPlaying;
	float volume;
	uint64_t previousTick;
};

