#include "Emulator.h"
#include "Renderer.h"
#include "SDL3/SDL.h"
#include <fstream>
#include <iostream>
#include <vector>

Emulator::Emulator(const std::string romPath, Renderer* renderer) : 
	romPath(romPath), 
	renderer(renderer),
	programCounter(0),
	indexRegister(0)
{
	
}

bool Emulator::Init()
{
	if (!LoadROM())
		return false;

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

	//SDL_Delay(1);

	return true;
}

void Emulator::Shutdown()
{

}

bool Emulator::LoadROM()
{
	std::cout << "Loading 'ROM/" << romPath << "'" << std::endl;

	// Open file
	std::ifstream file("ROM/" + romPath, std::ios::binary | std::ios::ate);
	if (!file)
	{
		std::cerr << "Could not open 'ROM/" << romPath << "'" << std::endl;
		return false;
	}

	// Get size, works due to std::ios::ate
	std::streamsize fileSize = file.tellg();

	// Rewind
	file.seekg(0, std::ios::beg);

	// Read file contents to
	std::vector<char> buffer(fileSize);
	if (file.read(buffer.data(), fileSize))
	{
		std::cout << "Loaded ROM..." << std::endl;
	}
	else
	{
		std::cerr << "Error, could not load 'ROM/" << romPath << "'" << std::endl;
		return false;
	}

	file.close();

	for (size_t i = 0; i < buffer.size(); i += 2)
	{
		printf("%02X%02X ", static_cast<unsigned char>(buffer[i]), static_cast<unsigned char>(buffer[i + 1]));
	}

	return true;
}
