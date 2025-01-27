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
	bool LoadROM();

	const std::string romPath;
	Renderer* renderer = nullptr;
	
	int programCounter;
	int indexRegister : 16;

};