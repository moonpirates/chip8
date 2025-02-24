// Copyright (c) 2025, Moonpirates. All rights reserved.
#pragma once

// Includes
#include <string>

// Forward declarations
class Window;
class Renderer;
class Emulator;
class Sound;

/**
 * @brief Main class for running the CHIP-8 emulation. 
 * 
 * A single instance of this class forms the entire lifetime of the application. It manages several subsystems: 
 * Emulator, Renderer and Sound as well the Window. Upon Init() it ensures all subsystems are correctly initialized,
 * otherwise quits the application. Then keeps Run() going, until user expresses the desire to quit, after which a
 * Shutdown() is called.
 * 
 * When the application isn't started through a ROM path in the arguments, initialization of Emulator and Sound is 
 * deferred until a ROM file is dragged on top of the Window. Until then, only Window and Renderer are active. 
 */
class Chip8
{
public:
	/**
	 * @brief Constructor, initialing without a ROM.
	 */
	Chip8();

	/**
	 * @brief Constructor, initializing with a ROM, immediately booting it. 
	 * @param romPath Path to the ROM file to load.
	 */
	Chip8(const std::string romPath);

	/**
	 * @brief Destructor
	 */
	~Chip8();

	/**
	 * @brief Initializes all required subsystems.
	 * @return Returns whether all subsystems have correctly initialized.
	 */
	bool Init();

	/**
	 * @brief Shuts down the Emulator and Sound subsystems. 
	 */
	void Shutdown();

	/**
	 * @brief Main loop, relaying the Run() to the Emulator, then the Renderer.
	 * @return Returns whether the application should still be running or not to the outside world.
	 */
	bool Run();

private:
	/**
	 * @brief Initializes the emulation process of a specific ROM.
	 * @return Returns whether the required systems correctly initialized (ie. whether the ROM was loaded correctly).
	 */
	bool InitROM();

	/**
	 * @brief Handles several input based events such as quiting or dragging a ROM file on top of the window.
	 * @return Returns false if the user desired to quit the application.
	 */
	bool HandleEvents();

	Window* window = nullptr;			///< Window instance.
	Renderer* renderer = nullptr;		///< Renderer subsystem instance.
	Sound* sound = nullptr;				///< Sound subsystem instance.
	Emulator* emulator = nullptr;		///< Emulator subsystem instance.

	std::string romPath;				///< Path to the current ROM CHIP-8 is currently emulating.
	bool running = false;				///< Boolean keeping track of whether the application should still be running.
	bool hasShutDown = false;			///< Fail-safe to prevent multiple Shutdown() calls.
};
