#pragma once

#include <cstdint>
#include <vector>

class Window;
struct SDL_Renderer;
union SDL_Event;

using namespace std;

class Renderer
{
public:
	Renderer(Window* window);

	bool Init();
	void Shutdown();
	void Render();

	void Clear();
	void Display(uint8_t x, uint8_t y, uint8_t n, uint16_t I, vector<uint8_t>& memory, vector<uint8_t>& vars);

private:
	static bool OnWindowEvent(void* data, SDL_Event* event);

	void UpdateRenderScale() const;
	void RenderBuffer();

	const int FRAMES_PER_SECOND = 60;

	Window* window = nullptr;
	SDL_Renderer* sdlRenderer = nullptr;
	vector<vector<bool>> screenBuffer; //TODO find a smarter way to read the pixel data from SDL as this is redundant information, https://youtu.be/EW1zXX89pfM?t=5109
	bool initialized = false;
	bool redraw = true; // 'true' because we want to draw one frame to clear the screen
	float nextRenderTime = 0.f;
};

