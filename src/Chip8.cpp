#include "Chip8.h"
#include "Window.h"
#include "Renderer.h"
#include "Emulator.h"
#include "Sound.h"

Chip8::Chip8() : romPath("")
{
}

Chip8::Chip8(const std::string romPath) : romPath(romPath)
{
}

Chip8::~Chip8()
{
	Shutdown();

	if (renderer != nullptr)
	{
		renderer->Shutdown();
		delete renderer;
		renderer = nullptr;
	}

	if (window != nullptr)
	{
		window->Shutdown();
		delete window;
		window = nullptr;
	}

}

bool Chip8::Init()
{
	// Setup base systems
	window = new Window();
	if (!window->Init())
		return false;

	renderer = new Renderer(window);
	if (!renderer->Init())
		return false;

	// Load ROM if it's been passed in through the constructor
	if (!romPath.empty() && !InitROM())
		return false;
	
	running = true;
	hasShutDown = false;

	return true;
}

void Chip8::Shutdown()
{
	if (hasShutDown)
		return;

	if (emulator != nullptr)
	{
		emulator->Shutdown();
		delete emulator;
		emulator = nullptr;
	}

	if (sound != nullptr)
	{
		sound->Shutdown();
		delete sound;
		sound = nullptr;
	}

	hasShutDown = true;
}

bool Chip8::Run()
{
	if (!HandleEvents())
		running = false;
	else if (emulator != nullptr)
		emulator->Run();

	renderer->Render();

	return running;
}

bool Chip8::InitROM()
{
	renderer->Clear();

	sound = new Sound();
	if (!sound->Init())
		return false;

	emulator = new Emulator(romPath, renderer, sound);
	if (!emulator->Init())
		return false;
}

bool Chip8::HandleEvents()
{
	SDL_Event e;
	if (SDL_PollEvent(&e))
	{
		if (e.type == SDL_EVENT_QUIT)
			return false;

		if (e.type == SDL_EVENT_KEY_UP && e.key.key == SDLK_ESCAPE)
			return false;

		if (e.type == SDL_EVENT_DROP_FILE)
		{
			Shutdown();
			romPath = e.drop.data;
			InitROM();
			hasShutDown = false;
		}
	}

	return true;
}
