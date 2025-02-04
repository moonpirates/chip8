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

class Emulator 
{
public:
	// Construction/destruction
	Emulator(const string romPath, Renderer* renderer, Sound* sound);

	// Core functionality
	bool Init();
	bool Run();
	void Shutdown();

private:
	// Initialization
	bool LoadROM();
	void LoadFont();

	// Core loop
	void HandleKeyboard();
	void HandleTimers();
	Opcode Fetch();
	void DecodeAndExecute(Opcode opcode);
	
	// Helper functions
	uint8_t GetOpcodeNibble(Opcode opcode, int nibbleIndex) const;

	// Config
	static const uint32_t PROGRAM_START = 0x200;
	static const uint32_t FONT_START = 0x50;
	static const uint32_t OPCODES_FREQUENCY = 700;
	static const uint32_t TIMER_DECREMENT_FREQUENCY = 60;
	static const vector<SDL_Scancode> KEY_MAP;

	// CHIP-8 core components
	vector<uint8_t> memory;
	vector<uint8_t> vars;
	uint16_t PC;
	uint16_t I;
	stack<uint16_t> stack;
	uint8_t delayTimer;
	uint8_t soundTimer;
	uint16_t keys;

	// Misc
	const string romPath;
	Renderer* renderer = nullptr;
	Sound* sound = nullptr;
	float nextOpcodeTime = 0.f;
	float nextTimerDecrementTime = 0.f;
};