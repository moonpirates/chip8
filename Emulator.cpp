#include "Emulator.h"
#include "Renderer.h"
#include "SDL3/SDL.h"
#include <fstream>
#include <cassert>

Emulator::Emulator(const string romPath, Renderer* renderer) : 
	romPath(romPath), 
	renderer(renderer),
	memory(4096, 0),
	PC(PROGRAM_START),
	I(0),
	vars(16, 0)
{
	
}

bool Emulator::Init()
{
	if (!LoadROM())
		return false;	
	
	LoadFont();

	return true;
}

bool Emulator::Run()
{
	SDL_Event e;
	if (SDL_PollEvent(&e))
	{
		if (e.type == SDL_EVENT_QUIT)
			return false;

		if (e.type == SDL_EVENT_KEY_UP && e.key.key == SDLK_ESCAPE)
			return false;
	}

	if (SDL_GetTicks() < nextOpcodeTime)
		return true;

	Opcode opcode = Fetch();
	DecodeAndExecute(opcode);
	nextOpcodeTime = SDL_GetTicks() + (1000.f / OPCODES_PER_SECOND);

	return true;
}

void Emulator::Shutdown()
{

}

bool Emulator::LoadROM()
{
	cout << "Loading 'ROM/" << romPath << "'" << endl;

	// Open file
	ifstream file("ROM/" + romPath, ios::binary | ios::ate);
	if (!file)
	{
		cerr << "Could not open 'ROM/" << romPath << "'" << endl;
		return false;
	}

	// Get size, works due to ios::ate
	streamsize fileSize = file.tellg();

	// Rewind
	file.seekg(0, ios::beg);

	// Reinterpret signed chars to unsigned chars, and start writing from PROGRAM_START in memory
	if (file.read(reinterpret_cast<char*>(&memory[PROGRAM_START]), fileSize))
	{
		cout << "Loaded ROM..." << endl;
	}
	else
	{
		cerr << "Error, could not load 'ROM/" << romPath << "'" << endl;
		return false;
	}

	// Close up
	file.close();

	for (size_t i = PROGRAM_START; i < PROGRAM_START + fileSize; i += 2)
	{
		printf("%02X%02X ", (memory[i]), (memory[i + 1]));
	}

	cout << endl << "--------------------------------------------------" << endl;

	return true;
}


void Emulator::LoadFont()
{
	const vector<uint8_t> FONT_DATA
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	memcpy(&memory[FONT_START], FONT_DATA.data(), FONT_DATA.size());
}

Opcode Emulator::Fetch()
{
	uint8_t opcodeA = memory[PC];
	uint8_t opcodeB = memory[PC + 1];

	PC += 2;

	return (opcodeA << 8) + opcodeB;

}

void Emulator::DecodeAndExecute(Opcode opcode)
{
	uint8_t x = GetOpcodeNibble(opcode, 1); // USE AS ADDRESS IN REGISTER, NOT BY VALUE
	uint8_t y = GetOpcodeNibble(opcode, 2); // USE AS ADDRESS IN REGISTER, NOT BY VALUE
	uint8_t n = opcode & 0xF;
	uint8_t nn = opcode & 0xFF;
	uint16_t nnn = opcode & 0xFFF;

	/*
	00E0 (clear screen)
	1NNN (jump)
	6XNN (set register VX)
	7XNN (add value to register VX)
	ANNN (set index register I)
	DXYN (display/draw)
	*/

	switch (GetOpcodeNibble(opcode, 0))
	{
		// 
		case 0x0:
			switch (nnn)
			{
				case 0x0E0:
					renderer->Clear();
					printf("[CLEAR]\n");
					break;
				default:
					printf("*** UNKNOWN CODE: %02X\n", opcode);
					break;
			}

			break;
		
		// Jump
		case 0x1:
			PC = nnn;
			printf("[SET PC] %02X\n", nnn);
			break;
		
		//case 0x2:
		//	break;
		//
		//case 0x3:
		//	break;
		//
		//case 0x4:
		//	break;
		//
		//case 0x5:
		//	break;
		
		// Set register
		case 0x6:
			vars[x] = nn;
			printf("[SET] vars[%d] = %d\n", x, nn);
			break;
		
		case 0x7:
			vars[x] += nn;
			printf("[INCREMENT] vars[%d] + %d\n", x, nn);
			break;
		
		//case 0x8:
		//	break;
		//
		//case 0x9:
		//	break;
		
		case 0xA:
			I = nnn;
			printf("[SET I] %02X\n", nnn);
			break;
		
		//case 0xB:
		//	break;
		//
		//case 0xC:
		//	break;
		
		case 0xD:
			renderer->Display(vars[x], vars[y], n, I, memory, vars);
			printf("[DISPLAY] x: %d y: %d, height: %d\n", vars[x], vars[y], n);
			break;
		
		//case 0xE:
		//	break;
		//
		//case 0xF:
		//	break;

		default:
			printf("*** UNKNOWN CODE: %02X\n", opcode);
			break;

	}

}

uint8_t Emulator::GetOpcodeNibble(Opcode opcode, int nibbleIndex) const
{
	assert(nibbleIndex <= 3);

	nibbleIndex = 3 - nibbleIndex;
	return (opcode >> nibbleIndex * 4) & 0xF; // Shift and mask last 4 bits
}
