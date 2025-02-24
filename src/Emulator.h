// Copyright (c) 2025, Moonpirates. All rights reserved.
#pragma once

// Includes
#include <string>
#include <vector>
#include <stack>

// Forward declarations
class Renderer;
class Sound;
enum SDL_Scancode;

// Usings
using Opcode = uint16_t;
using namespace std;

/**
 * @brief Emulator is responsible for loading and running CHIP-8 ROMs.
 * 
 * CHIP-8 ROM files consist of nothing but instructions which are dealt with during the Run() method. During a Run() we 
 * update our timers, handle input and handle our opcodes. This class works hand in hand with Renderer and Sound.
 */
class Emulator 
{
public:
	/**
	 * @brief Constructor
	 * @param romPath Path to the ROM this Emulator instance should run.
	 * @param renderer The Renderer we use to draw the visual part of our emulation.
	 * @param sound The Sound instance, used to play audio for aural part of our emulation.
	 */
	Emulator(const string romPath, Renderer* renderer, Sound* sound);

	/**
	 * @brief Initializes the Emulator, loading our ROM and font data into memory.
	 * @return Returns false if initialization fails.
	 */
	bool Init();
	
	/**
	 * @brief A single Run() cycle handles keyboard input, updates timers, and handles an opcode on a specific frequency.
	 */
	void Run();

private:
	/**
	 * @brief Tries to load a ROM from romPath.
	 * @return Returns whether loading was successful.
	 */
	bool LoadROM();

	/**
	 * @brief Loads font data CHIP-8 uses to render text into memory.
	 */
	void LoadFont();

	/**
	 * @brief Sets the correct bits for keys, based on KEY_MAP.
	 */
	void HandleKeyboard();

	/**
	 * @brief Updates the delay timer and sound timer. 
	 */
	void HandleTimers();

	/**
	 * @brief Fetches the currently relevant Opcode and increments the instruction pointer.
	 * @return The currently relevant Opcode.
	 */
	Opcode Fetch();

	/**
	 * @brief Core of the emulation process. It deals with the given Opcode, and acts accordingly. An overview of all
	 * opcodes can be found on https://en.wikipedia.org/wiki/CHIP-8#Opcode_table.
	 * @param opcode The Opcode which should be handled.
	 */
	void DecodeAndExecute(Opcode opcode);
	
	/**
	 * @brief Simple helper function, returning a nibble (4 bits) of a complete Opcode (16 bits). 
	 * @param opcode The Opcode to analyze.
	 * @param nibbleIndex The [0..3] (inclusive) nibble index of the 16 bits Opcode.
	 * @return Returns the value of the specified nibble.
	 */
	static uint8_t GetOpcodeNibble(Opcode opcode, int nibbleIndex);

	static const uint32_t PROGRAM_START = 0x200;			///< Start point in memory where ROM data is copied to.
	static const uint32_t FONT_START = 0x50;				///< Start point in memory where font data is copied to.
	static const uint32_t OPCODES_FREQUENCY = 700;			///< Number of opcodes that should be handled per second.
	static const uint32_t TIMER_DECREMENT_FREQUENCY = 60;	///< Frequency at which the timers should be decremented.
	static const vector<SDL_Scancode> KEY_MAP;				///< Mapping of SDL scan codes in a 0x0 to 0xF fashion.

	vector<uint8_t> memory;									///< CHIP-8's core internal memory.
	vector<uint8_t> vars;									///< CHIP-8's variable register.
	uint16_t PC = PROGRAM_START;							///< CHIP8's program counter, pointing to a specific instruction in memory.
	uint16_t I = 0;											///< CHIP8's index register, pointing to a specific memory location.
	stack<uint16_t> stack;									///< CHIP8's call stack, used for nested calls.
	uint8_t delayTimer = 0;									///< CHIP8's delay timer, used internally for timing events.
	uint8_t soundTimer = 0;									///< CHIP8's sound timer, which plays a sound when nonzero.
	uint16_t keys = 0;										///< Bitset of keys being pressed, ranging from [0xF..0x0].

	const string romPath;									///< Path of the ROM we're emulating.
	Renderer* renderer = nullptr;							///< Reference to the Renderer, used for Clear() and Display() opcodes.
	Sound* sound = nullptr;									///< Sound class, used to play audio when soundTimer > 0.
	float nextOpcodeTime = 0.f;								///< Internal clockwork for when the next Opcode should be dealt with.
	float nextTimerDecrementTime = 0.f;						///< Internal clockwork for when the timers should be decremented.
};
