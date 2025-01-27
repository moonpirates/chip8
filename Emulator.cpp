#include "Emulator.h"
#include "Renderer.h"
#include "SDL3/SDL.h"

Emulator::Emulator(const std::string romPath, Renderer* renderer) : romPath(romPath), renderer(renderer)
{

}

bool Emulator::Init()
{
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

	SDL_Delay(1);

	return true;
}

void Emulator::Shutdown()
{

}
