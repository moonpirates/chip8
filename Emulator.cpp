//#define CHIP8_ORIGINAL

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
	uint8_t x = GetOpcodeNibble(opcode, 1);
	uint8_t y = GetOpcodeNibble(opcode, 2);
	uint8_t n = opcode & 0xF;
	uint8_t nn = opcode & 0xFF;
	uint16_t nnn = opcode & 0xFFF;

	switch (GetOpcodeNibble(opcode, 0))
	{
		case 0x0:
		{	
			switch (nnn)
			{
				// Clears the screen.
				case 0x0E0:
				{
					renderer->Clear();
					printf("[CLEAR]\n");
					break;
				}

				// Returns from a subroutine.
				case 0xEE:
				{
					PC = stack.top();
					stack.pop();
					break;
				}

				default:
				{
					printf("*** UNKNOWN CODE: %02X\n", opcode);
					break;
				}
			}

			break;
		}
		
		// Jumps to address NNN
		case 0x1:
		{
			PC = nnn;
			printf("[SET PC] %02X\n", nnn);
			break;
		}
		
		// Calls subroutine at NNN
		case 0x2:
		{
			stack.push(PC);
			PC = nnn;
			break;
		}

		// Skips the next instruction if VX equals NN
		case 0x3:
		{
			if (vars[x] == nn)
			{
				PC += 2;
				printf("[SKIP] vars[%d] (%d) == %d\n", x, vars[x], nn);
			}
			break;
		}

		// Skips the next instruction if VX does not equal NN
		case 0x4:
		{
			if (vars[x] != nn)
			{
				PC += 2;
				printf("[SKIP] vars[%d] (%d) != %d\n", x, vars[x], nn);
			}
			break;
		}
		
		// Skips the next instruction if VX equals VY
		case 0x5:
		{
			if (vars[x] == vars[y])
			{
				PC += 2;
				printf("[SKIP] vars[%d] (%d) == vars[%d] (%d)\n", x, vars[x], y, vars[y]);
			}
			break;
		}

		// Sets VX to NN
		case 0x6:
		{
			vars[x] = nn;
			printf("[SET] vars[%d] = %d\n", x, nn);
			break;
		}
		
		// Adds NN to VX (carry flag is not changed)
		case 0x7:
		{
			vars[x] += nn;
			printf("[INCREMENT] vars[%d] + %d\n", x, nn);
			break;
		}
		
		case 0x8:
		{
			switch (n)
			{
				// Sets VX to the value of VY
				case 0x0:
				{
					vars[x] = vars[y];
					break;
				}

				// Sets VX to VX or VY. (bitwise OR operation)
				case 0x1:
				{
					vars[x] |= vars[y];
					break;
				}

				// Sets VX to VX and VY. (bitwise AND operation)
				case 0x2:
				{
					vars[x] &= vars[y];
					break;
				}

				// Sets VX to VX xor VY.
				case 0x3:
				{
					vars[x] ^= vars[y];
					break;
				}

				// Adds VY to VX. VF is set to 1 when there's an overflow, and to 0 when there is not.
				case 0x4:
				{
					// Overflow handling
					if (vars[x] + vars[y] > 255)
						vars[0xF] = 1;

					vars[x] += vars[y];

					break;
				}

				// VY is subtracted from VX. VF is set to 0 when there's an underflow, and 1 when there is not. 
				// (i.e. VF set to 1 if VX >= VY and 0 if not).
				case 0x5:
				{
					// Underflow handling
					vars[0xF] = vars[x] >= vars[y] ? 1 : 0; // TODO equation would already lead to 0 or 1?

					vars[x] = vars[x] - vars[y];
					break;
				}

				// Shifts VX to the right by 1, then stores the least significant bit of VX prior to the shift into VF.
				case 0x6:
				{
#ifdef CHIP8_ORIGINAL
					vars[x] = vars[y];
#endif
					// TODO double check if this is correct
					uint8_t shiftedBit = vars[x] & 0x1;
					vars[x] = vars[x] >> 1;
					vars[0xF] = shiftedBit;
					break;
				}

				// Sets VX to VY minus VX. VF is set to 0 when there's an underflow, and 1 when there is not. 
				// (i.e. VF set to 1 if VY >= VX).
				case 0x7:
				{
					vars[0xF] = vars[y] >= vars[x] ? 1 : 0; // TODO equation would already lead to 0 or 1?

					vars[x] = vars[y] - vars[x];
				}
					break;

				// Shifts VX to the left by 1, then sets VF to 1 if the most significant bit of VX prior to that 
				// shift was set, or to 0 if it was unset.
				case 0xE:
				{
#ifdef CHIP8_ORIGINAL
					vars[x] = vars[y];
#endif
					// TODO double check if this is correct
					uint8_t shiftedBit = vars[x] >> 7;
					vars[x] = vars[x] << 1;
					vars[0xF] = shiftedBit;
					break;
				}
				default:
				{
					printf("*** UNKNOWN CODE: %02X\n", opcode);
					break;
				}
			}
			break;
		}
		
		// Skips the next instruction if VX does not equal VY.
		case 0x9:
		{
			if (vars[x] != vars[y])
			{
				PC += 2;
				printf("[SKIP] vars[%d] (%d) != vars[%d] (%d)\n", x, vars[x], y, vars[y]);
			}
			break;
		}
		
		// Sets I to the address NNN.
		case 0xA:
		{
			I = nnn;
			printf("[SET I] %02X\n", nnn);
			break;
		}
		
		// Jumps to the address NNN plus V0.
		case 0xB:
		{
#ifdef CHIP8_ORIGINAL
			I = nnn + vars[0];
#else
			I = nnn + vars[x];
#endif
			break;
		}
		
		// Sets VX to the result of a bitwise and operation on a random number (Typically: 0 to 255) and NN.
		//case 0xC:
		// {
		//	break;
		// }
		
		// Draws a sprite at coordinate (VX, VY).
		case 0xD:
		{
			renderer->Display(vars[x], vars[y], n, I, memory, vars);
			printf("[DISPLAY] x: %d y: %d, height: %d\n", vars[x], vars[y], n);
			break;
		}
		
		// Key handling
		//case 0xE:
		// {
		//	break;
		// }

		case 0xF:
		{
			switch (nn)
			{
				// Sets the delay timer to VX.
				//case 0x15:
				//{
				//	break;
				//}

				// Sets the sound timer to VX.
				//case 0x18:
				//{
				//	break;
				//}
			
				// Adds VX to I. VF is not affected
				case 0x1E:
				{
					I += vars[x];
					break;
				}

				// Sets I to the location of the sprite for the character in VX(only consider the lowest nibble). 
				// Characters 0-F (in hexadecimal) are represented by a 4x5 font.
				case 0x29:
				{
					uint8_t characterIndex = memory[x] & 0xF; // Only grab last nibble
					characterIndex *= 0xFF * 0x5; // Get the right offset in memory
					I = memory[FONT_START + characterIndex];
					break;
				}

				// Stores the binary-coded decimal representation of VX, with the hundreds digit in memory at 
				// location in I, the tens digit at location I+1, and the ones digit at location I+2.
				case 0x33:
				{
					uint8_t var = vars[x];
					uint8_t ones = var % 10;
					var /= 10;
					uint8_t tens = var % 10;
					var /= 10;
					uint8_t hundreds = var % 10;

					memory[I] = hundreds;
					memory[I + 1] = tens;
					memory[I + 2] = ones;

					break;
				}

				// Stores from V0 to VX (including VX) in memory, starting at address I. The offset from I is 
				// increased by 1 for each value written, but I itself is left unmodified.
				case 0x55:
				{
					uint8_t len = vars[x];
					for (uint8_t i = 0; i <= len; i++)
#ifdef CHIP8_ORIGINAL
						memory[I++] = vars[i];
#else
						memory[I + i] = vars[i];
#endif

					break;
				}

				// Fills from V0 to VX (including VX) with values from memory, starting at address I. The offset 
				// from I is increased by 1 for each value read, but I itself is left unmodified.
				case 0x65:
				{
					uint8_t len = vars[x];

					for (uint8_t i = 0; i <= len; i++)
#ifdef CHIP8_ORIGINAL
						vars[i] = memory[I++];
#else
						vars[i] = memory[I + i];
#endif

					break;
				}

				default:
				{
					printf("*** UNKNOWN CODE: %02X\n", opcode);
					break;
				}
			}
			break;
		}

		default:
		{
			printf("*** UNKNOWN CODE: %02X\n", opcode);
			break;
		}

	}

}

uint8_t Emulator::GetOpcodeNibble(Opcode opcode, int nibbleIndex) const
{
	assert(nibbleIndex <= 3);

	nibbleIndex = 3 - nibbleIndex;
	return (opcode >> nibbleIndex * 4) & 0xF; // Shift and mask last 4 bits
}
