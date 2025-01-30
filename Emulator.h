#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <stack>

class Renderer;
enum SDL_Scancode;

using Opcode = uint16_t;
using namespace std;

class Emulator 
{
public:
	Emulator(const string romPath, Renderer* renderer);

	bool Init();
	bool Run();
	void Shutdown();

private:
	// Initialization
	bool LoadROM();
	void LoadFont();

	// Core loop
	bool HandleEvents();
	void HandleKeyboard();
	void HandleTimers();
	Opcode Fetch();
	void DecodeAndExecute(Opcode opcode);
	
	// Helper functions
	void PrintAsHex(unsigned char c) { cout << setw(2) << setfill('0') << hex << (int)c << dec << endl; }
	uint8_t GetOpcodeNibble(Opcode opcode, int nibbleIndex) const;

	static const uint32_t PROGRAM_START = 0x200;
	static const uint32_t FONT_START = 0x50;
	static const uint32_t OPCODES_FREQUENCY = 700;
	static const uint32_t TIMER_DECREMENT_FREQUENCY = 60;
	static const vector<SDL_Scancode> KEY_MAP;

	const string romPath;
	Renderer* renderer = nullptr;
	float nextOpcodeTime = 0.f;
	float nextTimerDecrementTime = 0.f;

	/*
	[X] memory: 4kb
	[X] display: 64x32
	[X] PC: program counter
	[X] I: 16 bit memory pointer
	[X] variables: 16x 8 bit (V0-VF)
	[X] stack: 16 bit addresses
	[X] delay timer: 8 bit
	[X] sound timer: 8 bit
	*/

	vector<uint8_t> memory;
	vector<uint8_t> vars;
	uint16_t PC;
	uint16_t I;
	stack<uint16_t> stack;
	uint8_t delayTimer;
	uint8_t soundTimer;
	uint16_t keys;
};