#include "Renderer.h"
#include "Window.h"
#include "SDL3/SDL.h"

Renderer::Renderer(Window* window) : window(window)
{
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

	// Clear screen
	//SDL_SetRenderDrawColor(sdlRenderer , 0, 0, 0, 255);
	//SDL_RenderClear(sdlRenderer);

	//// Place test pixels
	//for (float y = 0; y < window->GetCanvasHeight(); y++)
	//{
	//	for (float x = 0; x < window->GetCanvasWidth(); x++)
	//	{
	//		int r = (x / window->GetCanvasWidth()) * 255;
	//		int g = (y / window->GetCanvasHeight()) * 255;

	//		SDL_SetRenderDrawColor(sdlRenderer, r, g, 0, 255);
	//		SDL_RenderPoint(sdlRenderer, x, y);
	//	}
	//}

	// Render
	SDL_RenderPresent(sdlRenderer);
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

