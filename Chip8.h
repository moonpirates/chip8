#pragma once

#include <string>

class Window;
class Renderer;
class Emulator;

class Chip8
{
public:
	Chip8(const std::string romPath);
	~Chip8();

	bool Init();
	void Shutdown();
	bool Run();

private:
	Window* window = nullptr;
	Renderer* renderer = nullptr;
	Emulator* emulator = nullptr;

	const std::string romPath;
	bool running = false;
	bool hasShutDown = false;
};