#pragma once

#include <string>

class Window;
class Renderer;
class Emulator;
class Sound;

class Chip8
{
public:
	Chip8();
	Chip8(const std::string romPath);
	~Chip8();

	bool Init();
	void Shutdown();
	bool Run();

private:
	bool InitROM();
	bool HandleEvents();

	Window* window = nullptr;
	Renderer* renderer = nullptr;
	Sound* sound = nullptr;
	Emulator* emulator = nullptr;

	std::string romPath;
	bool running = false;
	bool hasShutDown = false;
};