// Copyright (c) 2025, Moonpirates. All rights reserved.
#pragma once

// Includes
#include <cstdint>
#include <vector>
#include "SDL3/SDL.h"

// Forward declarations
class Window;
struct SDL_Renderer;
struct SDL_GPUDevice;
struct SDL_GPUTexture;
struct SDL_GPUGraphicsPipeline;
struct SDL_GPUShader;
union SDL_Event;

// Usings
using namespace std;

/**
 * @brief The Renderer does all the visual lifting, providing an interface for Emulator to talk to.
 * 
 * The code works hand in hand with SDL's GPU framework, rendering the 64x32 screenBuffer vector to a series of quads,
 * which get rendered to a texture through the so called scenePipeline. This texture gets rendered as a single quad to
 * the screen through the postPipeline, where the post.frag.hlsl fragment shader does a bunch of post effects.
 */
class Renderer
{
public:
	/**
	 * @brief Constructor
	 * @param window Window in which the renderer needs to be created.
	 */
	Renderer(Window* window);

	/**
	 * @brief Initializes the Renderer, setting up dependencies such as the pipelines, samplers and static vertex info.
	 * @return Returns whether the Renderer initialized successfully. 
	 */
	bool Init();

	/**
	 * @brief Shuts down the Renderer and its dependencies.
	 */
	void Shutdown();

	/**
	 * @brief Renders the current state of the frame buffer at 60 frames per second, if a redraw is needed.
	 */
	void Render();

	/**
	 * @brief Clears the frame buffer. Intended for CHIP-8's 00E0 instruction.
	 */
	void Clear();

	/**
	 * @brief Modifies the frame buffer. Intended for CHIP-8's DXYN instruction. 
	 * @param x The X coordinate at which we should be drawing.
	 * @param y The Y coordinate at which we should be drawing.
	 * @param n Number of rows we should be drawing (height).
	 * @param I Start location in memory from which we should be drawing.
	 * @param memory Reference to the Emulator's memory.
	 * @param vars Reference to the Emulator's vars memory.
	 */
	void Display(uint8_t x, uint8_t y, uint8_t n, uint16_t I, vector<uint8_t>& memory, vector<uint8_t>& vars);

private:
	/**
	 * @brief Static callback for SDL's events, intended to enforce a redraw in case of window resizing.
	 * @param data Void pointer to Renderer instance triggering said event.
	 * @param event SDL's event struct containing event information.
	 * @return Returns whether method handled event successfully.
	 */
	static bool OnWindowEvent(void* data, SDL_Event* event);

	/**
	 * @brief Creates the SDL_GPUDevice used for rendering.
	 * @return Returns whether devices was acquired correctly.
	 */
	bool SetupDevice();

	/**
	 * @brief Sets up the SDL_GPUGraphicsPipeline used for rendering the CHIP-8 pixels to quads (scene).
	 * @return Returns whether pipeline was set up correctly.
	 */
	bool SetupScenePipeline();

	/**
	 * @brief Sets up the SDL_GPUGraphicsPipeline used for rendering the texture quad for post effects (post).
	 * @return Returns whether pipeline was set up correctly.
	 */
	bool SetupPostPipeline();

	/**
	 * @brief Loads a compiled vertex or fragment shader from disk.
	 * @param device SDL_GPUDevice used to poll which shader formats are supported.
	 * @param shaderFilename Filename of shader we're loading.
	 * @param samplerCount How many samplers the shader utilizes.
	 * @param uniformBufferCount How many uniforms the shader utilizes.
	 * @param storageBufferCount How many storage buffers the shader utilizes.
	 * @param storageTextureCount How many storage textures the shader utilizes.
	 * @return Returns the resulting SDL_GPUShader based on giving configuration.
	 */
	SDL_GPUShader* LoadShader(SDL_GPUDevice* device, const char* shaderFilename, Uint32 samplerCount, Uint32 uniformBufferCount, Uint32 storageBufferCount, Uint32 storageTextureCount);

	const int FRAMES_PER_SECOND = 60;					///< Target frames per second the Renderer tries to render at.
	const int NUM_VERTICES = 64 * 32 * 6;				///< Number of vertices needed to render all quads for the framebuffer.
														//TODO should rely on the configuration in Window

	Window* window = nullptr;							///< Reference to earlier created window in which the Renderer resides.
	SDL_GPUDevice* gpuDevice = nullptr;					///< The single SDL_GPUDevice used to render.
	SDL_GPUGraphicsPipeline* scenePipeline = nullptr;	///< SDL pipeline for rendering CHIP-8's pixels.
	SDL_GPUGraphicsPipeline* postPipeline = nullptr;	///< SDL pipeline for post effects.
	SDL_GPUBuffer* sceneVertexBuffer = nullptr;			///< Vertex buffer for the scene.
	SDL_GPUBuffer* postVertexBuffer = nullptr;			///< Vertex buffer for post effects.
	SDL_GPUBuffer* postIndexBuffer = nullptr;			///< Index buffer for the post effects.
	SDL_GPUTexture* sceneTexture = nullptr;				///< Texture to which the scene is rendered, utilized in post.
	SDL_GPUSampler* sampler = nullptr;					///< Texture sampler used to sample sceneTexture.
	
	vector<vector<bool>> screenBuffer;					///< 2D vector representing CHIP-8's pixels being either on or off.
	bool initialized = false;							///< Whether the Renderer is initialized.
	bool redraw = true;									///< Flipped to true when screenBuffer has been updated to enforce a redraw on the next Render()
														///< Initializes as 'true' so it automatically renders a clear frame.
	float nextRenderTime = 0.f;							///< Internal clockwork to keep track of when the next draw should be taking place.
};

