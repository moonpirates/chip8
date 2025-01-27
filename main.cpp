#include <iostream>
#include "Chip8.h"

int main(int argc, char* argv[])
{
	//TODO parse arguments to rom path
	Chip8 chip8 = Chip8("");
	
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