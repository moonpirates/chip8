// Copyright (c) 2025, Moonpirates. All rights reserved.
#pragma once

// Includes
#include "SDL3/SDL.h"
#include "SDL3/SDL_audio.h"

// Forward declarations
struct SDL_AudioStream;

/**
 * @brief Simple audio class, which plays beeps for a set amount of time.
 * 
 * AudioCallback() keeps a free running sinewave oscilator going, whose gate is opened when StartBeep() is called. The
 * callback guarantees the smallest (1ms) beeps are audible. By implementing small attack/decays on the beep, we don't
 * get any off axis popping of the audio.
 */
class Sound
{
public:

	/**
	 * @brief Fetches an SDL audio device and stream.
	 * @return Returns whether initialization was successful.
	 */
	bool Init();

	/**
	 * @brief Releases down SDL audio device and stream.
	 */
	void Shutdown();

	/**
	 * @brief Starts a beep for a specified duration in milliseconds.
	 * @param ms The amount of milliseconds the beep should be playing.
	 */
	void StartBeep(uint16_t ms) { audioEndTime = SDL_GetTicks() + ms; }
	
	/**
	 * @brief SDL's callback on the audio thread, doing the actual audio generation of the sine wave, gate handling and
	 * attack/decay handling.
	 * @param userdata Void pointer to this Sound instance.
	 * @param stream Audio stream in which we're generating our sine wave.
	 * @param additional_amount The amount of data, in bytes, that is needed right now.
	 * @param total_amount The total amount of data requested, in bytes, that is requested or available.
	 */
	static void AudioCallback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount);

private:
	static constexpr float BEEP_AMPLITUDE = 0.2f;			///< The amplitude at which the sine wave should be generated.
	static const int BEEP_FREQUENCY = 880;					///< The frequency of the sine wave.
	static const int FREQUENCY = 44100;						///< The sampling frequency of the device.
	static constexpr float ATTACK_DECAY_STEP_SIZE = 0.01f;	///< Step size in volume per sample to create a attack/decay.

	SDL_AudioDeviceID deviceID = 0;							///< The ID of the SDL_AudioDevice.
	SDL_AudioStream* stream = nullptr;						///< The stream into which we stream our samples.
	float samples[FREQUENCY] = {};							///< Our array of generated samples.
	uint32_t phase = 0;										///< Point along the sine wave we are, allowing for continuous wave generation.
	float volume = 0.f;										///< The audio gate as well as attack/decay multipliers.
	uint64_t audioEndTime = 0;								///< Point in time at which the current play should stop.
};

