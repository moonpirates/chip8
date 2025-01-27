#pragma once

#include <string>

class Renderer;

class Emulator 
{
public:
	Emulator(const std::string romPath, Renderer* renderer);

	bool Init();
	bool Run();
	void Shutdown();

private:
	const std::string romPath;
	Renderer* renderer = nullptr;
};