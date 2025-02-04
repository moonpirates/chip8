#include "Sound.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_audio.h"
#include <numbers>
#include <cmath>

using namespace std;

bool Sound::Init()
{
	// Open device
	deviceID = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
	
	// Open specs
	const SDL_AudioSpec spec = { SDL_AUDIO_F32, 1, FREQUENCY };
	stream = SDL_OpenAudioDeviceStream(deviceID, &spec, AudioCallback, this);

	// Quirk of SDL, starts in paused mode
	SDL_ResumeAudioStreamDevice(stream);
	
	if (stream == nullptr)
	{
		SDL_Log("Could not create stream: %s", SDL_GetError());
		return false;
	}

	return true;
}

void Sound::Shutdown()
{
	SDL_DestroyAudioStream(stream);
	SDL_CloseAudioDevice(deviceID);
}

void Sound::AudioCallback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount)
{
	// additional_amount 	the amount of data, in bytes, that is needed right now.
	// total_amount 		the total amount of data requested, in bytes, that is requested or available.
	Sound* sound = static_cast<Sound*>(userdata);

	// Determine whether beep gate should be open
	bool shouldBePlaying = SDL_GetTicks() < sound->audioEndTime;
	
	// Amount arguments are in bytes
	const int numFloats = additional_amount / sizeof(float);

	// Generate sine samples
	for (size_t i = 0; i < numFloats; i++)
	{
		// Short attack/decay to prevent pops from off axis sines
		const float ATTACK_DECAY_STEP_SIZE = 0.01f;
		if (shouldBePlaying && sound->volume < 1.f)
			sound->volume = std::min(sound->volume + ATTACK_DECAY_STEP_SIZE, 1.f);
		else if (!shouldBePlaying && sound->volume > 0.f)
			sound->volume = std::max(sound->volume - ATTACK_DECAY_STEP_SIZE, 0.f);

		// Generate sine multiplied by the desired amplitude and volume modifier
		sound->samples[i] = std::sin(sound->phase * 2 * numbers::pi / sound->FREQUENCY) * BEEP_AMPLITUDE * sound->volume;
		sound->phase += BEEP_FREQUENCY;
	}
	
	// Put generated waveform into stream
	SDL_PutAudioStreamData(stream, &sound->samples, additional_amount);
}

