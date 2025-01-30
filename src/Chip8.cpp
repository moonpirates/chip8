#include "Chip8.h"
#include "Window.h"
#include "Renderer.h"
#include "Emulator.h"
#include "Sound.h"

Chip8::Chip8(const std::string romPath) : romPath(romPath)
{
}

Chip8::~Chip8()
{
	Shutdown();
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

	sound = new Sound();
	if (!sound->Init())
		return false;

	emulator = new Emulator(romPath, renderer, sound);
	if (!emulator->Init())
		return false;
	
	running = true;
	hasShutDown = false;

	return true;
}

void Chip8::Shutdown()
{
	if (hasShutDown)
		return;

	emulator->Shutdown();
	sound->Shutdown();
	renderer->Shutdown();
	window->Shutdown();

	delete emulator;
	delete sound;
	delete renderer;
	delete window;

	emulator = nullptr;
	sound = nullptr;
	renderer = nullptr;
	window = nullptr;

	hasShutDown = true;
}

bool Chip8::Run()
{
	if (!emulator->Run())
		running = false;
	else
		renderer->Render();

	return running;
}
