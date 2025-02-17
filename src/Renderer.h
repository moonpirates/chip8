#pragma once

#include <cstdint>
#include <vector>
#include "SDL3/SDL.h"

class Window;
struct SDL_Renderer;
struct SDL_GPUDevice;
struct SDL_GPUTexture;
struct SDL_GPUGraphicsPipeline;
struct SDL_GPUShader;
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

	bool SetupDevice();
	bool SetupScenePipeline();
	bool SetupPostPipeline();
	SDL_GPUShader* LoadShader(SDL_GPUDevice* device, const char* shaderFilename, Uint32 samplerCount, Uint32 uniformBufferCount, Uint32 storageBufferCount, Uint32 storageTextureCount);

	void UpdateRenderScale() const;
	void RenderBuffer();

	const int FRAMES_PER_SECOND = 60;
	const int NUM_VERTICES = 64 * 32 * 6;

	Window* window = nullptr;
	SDL_GPUDevice* gpuDevice = nullptr;
	SDL_GPUGraphicsPipeline* scenePipeline = nullptr;
	SDL_GPUGraphicsPipeline* postPipeline = nullptr;
	SDL_GPUBuffer* sceneVertexBuffer = nullptr;
	SDL_GPUBuffer* postVertexBuffer = nullptr;
	SDL_GPUBuffer* postIndexBuffer = nullptr;
	SDL_GPUTexture* sceneTexture;
	SDL_GPUSampler* sampler;
	
	vector<vector<bool>> screenBuffer; //TODO find a smarter way to read the pixel data from SDL as this is redundant information, https://youtu.be/EW1zXX89pfM?t=5109
	bool initialized = false;
	bool redraw = true; // 'true' because we want to draw one frame to clear the screen
	float nextRenderTime = 0.f;
};

