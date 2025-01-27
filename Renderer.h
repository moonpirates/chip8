#pragma once

class Window;
struct SDL_Renderer;

class Renderer
{
public:
	Renderer(Window* window);

	bool Init();
	void Shutdown();
	void Render();

private:
	void UpdateRenderScale() const;

	Window* window = nullptr;
	SDL_Renderer* sdlRenderer = nullptr;
};

