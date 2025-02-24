// Copyright (c) 2025, Moonpirates. All rights reserved.
#pragma once

// Forward declares
struct SDL_Window;

/**
 * @brief Simple subsystem wrapper around an SDL_Window.
 */
class Window
{
public:
	/**
	 * @brief Initializes SDL, as well as an SDL_Window.
	 * @return Returns whether initialization was successful.
	 */
	bool Init();

	/**
	 * @brief Shuts down the SDL_Window and SDL.
	 */
	void Shutdown();

	/**
	 * @brief Gets the width of the canvas (CHIP-8's amount of horizontal pixels).
	 * @return Returns the width of the canvas.
	 */
	constexpr int GetCanvasWidth() const { return CANVAS_WIDTH; }

	/**
	 * @brief Gets the height of the canvas (CHIP-8's amount of vertical pixels).
	 * @return Returns the height of the canvas.
	 */
	constexpr int GetCanvasHeight() const { return CANVAS_HEIGHT; }

	/**
	 * @brief Gets the SDL_Window instance which this class wraps.
	 * @return Returns underlying SDL_Window.
	 */
	SDL_Window* GetSDLWindow() const { return sdlWindow; }

private:
	SDL_Window* sdlWindow;							///< The SDL_Window instance which this class wraps.

	static const int CANVAS_WIDTH = 64;				///< The number of horizontal pixels the CHIP-8 canvas holds.
	static const int CANVAS_HEIGHT = 32;			///< The number of vertical pixels the CHIP-8 canvas holds.
	static const int INITIAL_SCALE_FACTOR = 15;		///< Multiplier used to determine the initial size of the window.
};

