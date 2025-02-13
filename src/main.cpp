#include <iostream>
#include <filesystem>
#include "Chip8.h"

int main(int argc, const char* argv[])
{
	Chip8* chip8 = nullptr;

	if (argc < 2)
		chip8 = new Chip8("ROM/IBM Logo.ch8");
		//chip8 = new Chip8("ROM/BC_test.ch8");
		//chip8 = new Chip8("ROM/test_opcode.ch8");
		//chip8 = new Chip8("ROM/test_opcode_with_audio.ch8");
		//chip8 = new Chip8("ROM/breakout.rom");
		//chip8 = new Chip8("ROM/snake.ch8");
		//chip8 = new Chip8("ROM/keypad.ch8");
		//chip8 = new Chip8("ROM/pong2.ch8");
		//chip8 = new Chip8();
	else if (argc == 2)
		chip8 = new Chip8(argv[1]);
	else
	{
		std::cout << "Usage: " << std::filesystem::path(argv[0]).filename().string() << " [ROM path]" << std::endl;
		std::cout << "Optionally, you can also drag the ROM file onto the window." << std::endl;
		return -1;
	}
	
	// Init
	if (!chip8->Init())
		return -1;

	// Main loop
	while (chip8->Run())
	{
		// ...
	}

	// Shutdown 
	chip8->Shutdown();

	return 0;
}