#include "Window.h"
#include "SDL3/SDL.h"

bool Window::Init()
{
	// Init SDL
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO))
	{
		SDL_Log("Could not init SDL: %s", SDL_GetError());
		return false;
	}

	// Init window
	sdlWindow = SDL_CreateWindow("CHIP-8 Emulator", CANVAS_WIDTH, CANVAS_HEIGHT, SDL_WINDOW_RESIZABLE);
	if (sdlWindow == nullptr)
	{
		SDL_Log("Could not create window: %s", SDL_GetError());
		return false;
	}

	// Set an initial window size
	SDL_SetWindowSize(sdlWindow, CANVAS_WIDTH * INITIAL_SCALE_FACTOR, CANVAS_HEIGHT * INITIAL_SCALE_FACTOR);

	return true;
}

void Window::Shutdown()
{
	SDL_DestroyWindow(sdlWindow);
	SDL_Quit();
}
