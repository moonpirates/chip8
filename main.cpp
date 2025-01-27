#include <iostream>
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

static bool gShutDown;
const int WINDOW_WIDTH = 64;
const int WINDOW_HEIGHT = 32;
const int INITIAL_SCALE_FACTOR = 10;

SDL_Window* gSDLWindow;
SDL_Renderer* gSDLRenderer;

bool update()
{
	SDL_Event e;
	if (SDL_PollEvent(&e))
	{
		if (e.type == SDL_EVENT_QUIT)
			return false;
		
		if (e.type == SDL_EVENT_KEY_UP && e.key.key == SDLK_ESCAPE)
			return false;
	}

	SDL_Delay(2);
	
	return true;
}

void updateRenderScale()
{
	int currentWidth;
	int currentHeight;
	SDL_GetWindowSize(gSDLWindow, &currentWidth, &currentHeight);
	const float scaleX = currentWidth / (float)WINDOW_WIDTH;
	const float scaleY = currentHeight / (float)WINDOW_HEIGHT;
	SDL_SetRenderScale(gSDLRenderer, scaleX, scaleY);
}

void render(Uint64 aTicks)
{
	updateRenderScale();

	// Clear screen
	SDL_SetRenderDrawColor(gSDLRenderer, 0, 0, 0, 255);
	SDL_RenderClear(gSDLRenderer);

	// Place test pixels
	for (int y = 0; y < WINDOW_HEIGHT; y++)
	{
		for (int x = 0; x < WINDOW_WIDTH; x++)
		{
			int r = (x / (float)WINDOW_WIDTH) * 255;
			int g = (y / (float)WINDOW_HEIGHT) * 255;

			SDL_SetRenderDrawColor(gSDLRenderer, r, g, 0, 255);
			SDL_RenderPoint(gSDLRenderer, x, y);
		}
	}

	// Render
	SDL_RenderPresent(gSDLRenderer);
}

void loop()
{
	if (!update())
		gShutDown = true;
	else
		render(SDL_GetTicks());
}

int main(int argc, char* argv[])
{
	// Init
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
		return -1;

	if (!SDL_CreateWindowAndRenderer("CHIP-8 Emulator", WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_RESIZABLE, &gSDLWindow, &gSDLRenderer))
	{
		SDL_Log("Could not create window or renderer: %s", SDL_GetError());
		return -1;
	}

	SDL_SetWindowSize(gSDLWindow, WINDOW_WIDTH * INITIAL_SCALE_FACTOR, WINDOW_HEIGHT * INITIAL_SCALE_FACTOR);

	// Main loop
	gShutDown = false;
	while (!gShutDown)
		loop();

	// Shutdown
	SDL_DestroyRenderer(gSDLRenderer);
	SDL_DestroyWindow(gSDLWindow);
	SDL_Quit();

	return 0;
}