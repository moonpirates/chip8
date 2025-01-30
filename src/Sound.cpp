#define _USE_MATH_DEFINES
#include "Sound.h"
#include "SDL3/SDL.h"
#include "SDL3/SDL_audio.h"
#include <cmath>
#include <iostream>

using namespace std;

const int AMPLITUDE = 28000;
const int FREQUENCY = 44100 / 2;

Sound::Sound()
{
}

bool Sound::Init()
{
	const SDL_AudioSpec srcspec = { SDL_AUDIO_S16, 1, 22050 };
	const SDL_AudioSpec dstspec = { SDL_AUDIO_F32, 2, 48000 };

	SDL_AudioStream* audioStream = SDL_CreateAudioStream(&srcspec, &dstspec);

	if (audioStream == nullptr)
	{
		SDL_Log("Could not create renderer: %s", SDL_GetError());
		return false;
	}

	Sint16 samples[1024];
	
	for (size_t i = 0; i < 1024; i++)
	{
		samples[i] = AMPLITUDE * std::sin(i * 2 * 3.14159265358979323846 / FREQUENCY);
	}

	int rc = SDL_PutAudioStreamData(audioStream, samples, 1024 * sizeof(Sint16));

	if (rc == -1)
	{
		cerr << "ruh oh" << endl;
	}
}

void Sound::Shutdown()
{

}
