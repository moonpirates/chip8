#pragma once

struct SDL_Window;

class Window
{
public:
	bool Init();
	void Shutdown();

	constexpr int GetCanvasWidth() const { return CANVAS_WIDTH; }
	constexpr int GetCanvasHeight() const { return CANVAS_HEIGHT; }

	SDL_Window* GetSDLWindow() const { return sdlWindow; } //TODO prevent exposing SDL by using Window as a factory for Renderer

private:
	SDL_Window* sdlWindow;

	const int CANVAS_WIDTH = 64;
	const int CANVAS_HEIGHT = 32;
	const int INITIAL_SCALE_FACTOR = 10;
};

