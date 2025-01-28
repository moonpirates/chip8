#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <stack>

class Renderer;

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
	Opcode Fetch();
	void DecodeAndExecute(Opcode opcode);
	
	// Helper functions
	void PrintAsHex(unsigned char c) { cout << setw(2) << setfill('0') << hex << (int)c << dec << endl; }
	uint8_t GetOpcodeNibble(Opcode opcode, int nibbleIndex) const;

	const int PROGRAM_START = 0x200;
	const int FONT_START = 0x50;
	const int OPCODES_PER_SECOND = 700;

	const string romPath;
	Renderer* renderer = nullptr;
	float nextOpcodeTime = 0.f;

	/*
	[X] memory: 4kb
	[X] display: 64x32
	[X] PC: program counter
	[X] I: 16 bit memory pointer
	[X] variables: 16x 8 bit (V0-VF)
	[ ] stack: 16 bit addresses
	[ ] delay timer: 8 bit
	[ ] sound timer: 8 bit
	*/

	vector<uint8_t> memory;
	vector<uint8_t> vars;
	uint16_t PC;
	uint16_t I;
	stack<uint16_t> stack; //TODO stack capacity of 16 should suffice
};