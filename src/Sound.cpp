#include "Sound.h"
#include <numbers>
#include <cmath>
#include "SDL3/SDL.h"
#include "SDL3/SDL_audio.h"

using namespace std;

Sound::Sound() : deviceID(0), v(0.f), isPlaying(false), volume(0.f), previousTick(0)
{
}

bool Sound::Init()
{
	// Open device
	deviceID = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
	
	// Open specs
	const SDL_AudioSpec spec = { SDL_AUDIO_F32, 1, FREQUENCY };
	SDL_AudioStream* stream = SDL_OpenAudioDeviceStream(deviceID, &spec, AudioCallback, this);

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
	SDL_CloseAudioDevice(deviceID);
}

void Sound::AudioCallback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount)
{
	// additional_amount 	the amount of data, in bytes, that is needed right now.
	// total_amount 		the total amount of data requested, in bytes, that is requested or available.
	Sound* sound = static_cast<Sound*>(userdata);
	
	// Calculate step size in volume
	const uint64_t currentTick = SDL_GetTicks();
	const float deltaTime = (currentTick - sound->previousTick);
	const float step = deltaTime / ATTACK_DECAY_DURATION_MS * BEEP_AMPLITUDE;

	// Either attack or decay
	if (sound->isPlaying && sound->volume < BEEP_AMPLITUDE)
		sound->volume = min(sound->volume + step, BEEP_AMPLITUDE);
	else if (!sound->isPlaying && sound->volume > 0.f)
		sound->volume = max(sound->volume - step, 0.f);

	//cout << "vol: " << sound->volume << ",   \ttime: " << (currentTick / 1000.f) << ",\tdeltaTime: " << deltaTime << ",\tstep: " << step << endl;

	// Amount arguments are in bytes
	const int numFloats = additional_amount / sizeof(float);
	for (size_t i = 0; i < numFloats; i++)
	{
		// https://gist.github.com/bashkirtsevich/c0c1992a1cdcc57add02b6c7cc783ab1
		sound->samples[i] = std::sin(sound->v * 2 * numbers::pi / sound->FREQUENCY) * sound->volume;
		sound->v += BEEP_FREQUENCY;

		//TODO investigate weird pitch shift around 10 secs
	}

	// Put generated waveform into stream
	SDL_PutAudioStreamData(stream, &sound->samples, additional_amount);

	sound->previousTick = currentTick;
}
