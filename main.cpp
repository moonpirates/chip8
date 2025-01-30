#include <iostream>
#include "Chip8.h"

int main(int argc, char* argv[])
{
	//TODO parse arguments to rom path
	//Chip8 chip8 = Chip8("IBM Logo.ch8");
	//Chip8 chip8 = Chip8("BC_test.ch8");
	//Chip8 chip8 = Chip8("test_opcode.ch8");
	Chip8 chip8 = Chip8("test_opcode_with_audio.ch8");
	//Chip8 chip8 = Chip8("breakout.rom");
	//Chip8 chip8 = Chip8("snake.ch8");
	//Chip8 chip8 = Chip8("keypad.ch8");
	//Chip8 chip8 = Chip8("pong2.ch8");
	
	
	// Init
	if (!chip8.Init())
		return -1;

	// Main loop
	while (chip8.Run())
	{
		// ...
	}

	// Shutdown 
	chip8.Shutdown();

	return 0;
}