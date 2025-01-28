#include "Renderer.h"
#include "Window.h"
#include "SDL3/SDL.h"
#include <iostream>

Renderer::Renderer(Window* window) : window(window)
{
	screenBuffer = vector<vector<bool>>(window->GetCanvasWidth(), vector<bool>(window->GetCanvasHeight(), false));
}

bool Renderer::Init()
{
	sdlRenderer = SDL_CreateRenderer(window->GetSDLWindow(), nullptr);
	if (sdlRenderer == nullptr)
	{
		SDL_Log("Could not create renderer: %s", SDL_GetError());
		return false;
	}

	return true;
}

void Renderer::Shutdown()
{
	SDL_DestroyRenderer(sdlRenderer);
}

void Renderer::Render()
{
	//TODO only call after resize event, see https://stackoverflow.com/a/40693139/9763789
	UpdateRenderScale();

	if (SDL_GetTicks() < nextRenderTime)
		return;

	// Render
	if (redraw)
	{
		RenderBuffer();
		SDL_RenderPresent(sdlRenderer);
		redraw = false;
	}

	nextRenderTime = SDL_GetTicks() + (1000.f / FRAMES_PER_SECOND);
}

void Renderer::Clear()
{
	// Clear screen
	SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
	SDL_RenderClear(sdlRenderer);

	screenBuffer = vector<vector<bool>>(window->GetCanvasWidth(), vector<bool>(window->GetCanvasHeight(), false));
}

void Renderer::Display(uint8_t x, uint8_t y, uint8_t n, uint16_t I, vector<uint8_t>& memory, vector<uint8_t>& vars)
{
	x = x % window->GetCanvasWidth();
	const uint8_t originalX = x;
	y = y % window->GetCanvasHeight();

	vars[0xF] = 0;

	for (uint8_t row = 0; row < n && y < window->GetCanvasHeight(); row++, y++)
	{
		const uint8_t spriteData = memory[I + row];
		x = originalX;

		for (int col = 7; col >= 0 && x < window->GetCanvasWidth(); col--, x++)
		{
			bool currentPixel = screenBuffer[x][y];
			bool newPixel = (spriteData & (1 << col)); // Grab the relevant bit from spriteData

			if (newPixel && currentPixel)
				vars[0xF] = 1;
			
			newPixel ^= currentPixel;

			screenBuffer[x][y] = newPixel;
		}
	}

	redraw = true;
}

void Renderer::UpdateRenderScale() const
{
	// Get our window's current width/height
	int currentWidth;
	int currentHeight;
	SDL_GetWindowSize(window->GetSDLWindow(), &currentWidth, &currentHeight);

	// Calculate the scale of canvas in relation to our window
	const float scaleX = currentWidth / (float)window->GetCanvasWidth();
	const float scaleY = currentHeight / (float)window->GetCanvasHeight();
	
	// Update render scale
	SDL_SetRenderScale(sdlRenderer, scaleX, scaleY);
}

void Renderer::RenderBuffer()
{
	for (int y = 0; y < window->GetCanvasHeight(); y++)
	{
		for (int x = 0; x < window->GetCanvasWidth(); x++)
		{
			const int value = screenBuffer[x][y] ? 255 : 0;
			SDL_SetRenderDrawColor(sdlRenderer, value, value, value, 255);
			SDL_RenderPoint(sdlRenderer, x, y);
		}
	}
}

