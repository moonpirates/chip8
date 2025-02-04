#pragma once
#include "SDL3/SDL.h"
#include "SDL3/SDL_audio.h"

struct SDL_AudioStream;

class Sound
{
public:
	bool Init();
	void Shutdown();
	void StartBeep(uint16_t ms) { audioEndTime = SDL_GetTicks() + ms; }
	
	static void AudioCallback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount);

private:
	static constexpr float BEEP_AMPLITUDE = 0.2f;
	static const int BEEP_FREQUENCY = 880;
	static const int FREQUENCY = 44100;

	SDL_AudioDeviceID deviceID = 0;
	SDL_AudioStream* stream = nullptr;
	float samples[FREQUENCY] = {};
	uint32_t phase = 0;
	float volume = 0.f;
	uint64_t audioEndTime = 0;
};

