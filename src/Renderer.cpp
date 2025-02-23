#include "Renderer.h"
#include "Window.h"
#include <iostream>
#include "SDL3/SDL_events.h"
#include "SDL3/SDL_gpu.h"

typedef struct PositionColorVertex
{
	float x, y, z;
	Uint8 r, g, b, a;
} PositionColorVertex;

typedef struct PositionTextureVertex
{
	float x, y, z;
	float u, v;
} PositionTextureVertex;

typedef struct ShaderUniform
{
	float w;
	float h;
} ShaderUniform;


Renderer::Renderer(Window* window) : window(window)
{
	screenBuffer = vector<vector<bool>>(window->GetCanvasWidth(), vector<bool>(window->GetCanvasHeight(), false));
}

bool Renderer::Init()
{
	if (!SetupDevice())
		return false;

	if (!SetupScenePipeline())
		return false;

	if (!SetupPostPipeline())
		return false;

	// Create texture
	SDL_Point windowSize{};
	SDL_GetWindowSize(window->GetSDLWindow(), &windowSize.x, &windowSize.y);

	SDL_GPUTextureCreateInfo sceneTextureCreateInfo{};
	sceneTextureCreateInfo.type = SDL_GPU_TEXTURETYPE_2D,
	sceneTextureCreateInfo.width = windowSize.x;
	sceneTextureCreateInfo.height = windowSize.y;
	sceneTextureCreateInfo.layer_count_or_depth = 1;
	sceneTextureCreateInfo.num_levels = 1;
	sceneTextureCreateInfo.sample_count = SDL_GPU_SAMPLECOUNT_1;
	sceneTextureCreateInfo.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	sceneTextureCreateInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER | SDL_GPU_TEXTUREUSAGE_COLOR_TARGET;
	sceneTexture = SDL_CreateGPUTexture(gpuDevice, &sceneTextureCreateInfo);

	// Create sampler
	SDL_GPUSamplerCreateInfo samplerCreateInfo{};
	samplerCreateInfo.min_filter = SDL_GPU_FILTER_LINEAR;
	samplerCreateInfo.mag_filter = SDL_GPU_FILTER_LINEAR;
	samplerCreateInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
	samplerCreateInfo.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	samplerCreateInfo.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	samplerCreateInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
	sampler = SDL_CreateGPUSampler(gpuDevice, &samplerCreateInfo);

	// Create vertex/index buffers
	SDL_GPUBufferCreateInfo sceneVertexBufferCreateInfo{};
	sceneVertexBufferCreateInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
	sceneVertexBufferCreateInfo.size = sizeof(PositionColorVertex) * NUM_VERTICES;
	sceneVertexBuffer = SDL_CreateGPUBuffer(gpuDevice, &sceneVertexBufferCreateInfo);

	SDL_GPUBufferCreateInfo postVertexBufferCreateInfo{};
	postVertexBufferCreateInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
	postVertexBufferCreateInfo.size = sizeof(PositionTextureVertex) * 4;
	postVertexBuffer = SDL_CreateGPUBuffer(gpuDevice, &postVertexBufferCreateInfo);

	SDL_GPUBufferCreateInfo postIndexBufferCreateInfo{};
	postIndexBufferCreateInfo.usage = SDL_GPU_BUFFERUSAGE_INDEX;
	postIndexBufferCreateInfo.size = sizeof(Uint16) * 6;
	postIndexBuffer = SDL_CreateGPUBuffer(gpuDevice, &postIndexBufferCreateInfo);

	// Transfer static information
	SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo{};
	transferBufferCreateInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	transferBufferCreateInfo.size = (sizeof(PositionTextureVertex) * 4) + (sizeof(Uint16) * 6);
	SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(gpuDevice, &transferBufferCreateInfo);

	PositionTextureVertex* transferData = (PositionTextureVertex*)SDL_MapGPUTransferBuffer(gpuDevice, transferBuffer, false);
	transferData[0] = { -1,  1, 0, 0, 0 };
	transferData[1] = {  1,  1, 0, 1, 0 };
	transferData[2] = {  1, -1, 0, 1, 1 };
	transferData[3] = { -1, -1, 0, 0, 1 };

	Uint16* indexData = (Uint16*)&transferData[4];
	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;
	indexData[3] = 0;
	indexData[4] = 2;
	indexData[5] = 3;

	SDL_UnmapGPUTransferBuffer(gpuDevice, transferBuffer);

	SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(gpuDevice);
	SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);
	
	SDL_GPUTransferBufferLocation vertexTransferBufferLocation{};
	vertexTransferBufferLocation.transfer_buffer = transferBuffer;
	vertexTransferBufferLocation.offset = 0;

	SDL_GPUBufferRegion vertexBufferRegion{};
	vertexBufferRegion.buffer = postVertexBuffer;
	vertexBufferRegion.offset = 0;
	vertexBufferRegion.size = sizeof(PositionTextureVertex) * 4;

	SDL_UploadToGPUBuffer(copyPass, &vertexTransferBufferLocation, &vertexBufferRegion, false);

	SDL_GPUTransferBufferLocation indexTransferBufferLocation{};
	indexTransferBufferLocation.transfer_buffer = transferBuffer;
	indexTransferBufferLocation.offset = sizeof(PositionTextureVertex) * 4;

	SDL_GPUBufferRegion indexBufferRegion{};
	indexBufferRegion.buffer = postIndexBuffer;
	indexBufferRegion.offset = 0;
	indexBufferRegion.size = sizeof(Uint16) * 6;

	SDL_UploadToGPUBuffer(copyPass, &indexTransferBufferLocation, &indexBufferRegion, false);

	SDL_EndGPUCopyPass(copyPass);
	SDL_SubmitGPUCommandBuffer(commandBuffer);
	SDL_ReleaseGPUTransferBuffer(gpuDevice, transferBuffer);

	// Watch window resizing
	SDL_AddEventWatch(OnWindowEvent, this);

	initialized = true;

	return true;
}

void Renderer::Shutdown()
{
	if (!initialized)
		return;

	SDL_RemoveEventWatch(OnWindowEvent, this);

	SDL_ReleaseWindowFromGPUDevice(gpuDevice, window->GetSDLWindow());
	SDL_DestroyGPUDevice(gpuDevice);
	//SDL_DestroyRenderer(sdlRenderer);
}

void Renderer::Render()
{
	UpdateRenderScale();

	if (SDL_GetTicks() < nextRenderTime)
		return;

	// Render
	if (redraw)
	{
		SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(gpuDevice);
		if (commandBuffer == nullptr)
		{
			SDL_Log("Failed to acquire command buffer: %s", SDL_GetError());
			return;
		}

		SDL_GPUTexture* swapchainTexture;
		if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, window->GetSDLWindow(), &swapchainTexture, nullptr, nullptr))
		{
			SDL_Log("Failed to acquire swapchain texture: %s", SDL_GetError());
			return;
		}

		if (swapchainTexture != nullptr)
		{
			////////////////////////////// SCENE RENDER PASS //////////////////////////////
			
			SDL_GPUColorTargetInfo sceneTargetInfo = { 0 };
			sceneTargetInfo.texture = sceneTexture;
			sceneTargetInfo.clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
			sceneTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
			sceneTargetInfo.store_op = SDL_GPU_STOREOP_STORE;			

			// Bind pipeline
			SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &sceneTargetInfo, 1, nullptr);
			SDL_BindGPUGraphicsPipeline(renderPass, scenePipeline);
			
			// Bind vertex buffer
			SDL_GPUBufferBinding bufferBinding{};
			bufferBinding.buffer = sceneVertexBuffer;
			bufferBinding.offset = 0;

			SDL_BindGPUVertexBuffers(renderPass, 0, &bufferBinding, 1);

			// Create transfer buffer
			SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo{};
			transferBufferCreateInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
			transferBufferCreateInfo.size = sizeof(PositionColorVertex) * NUM_VERTICES;

			SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(gpuDevice, &transferBufferCreateInfo);
			PositionColorVertex* transferData = (PositionColorVertex*)SDL_MapGPUTransferBuffer(gpuDevice, transferBuffer, false);

			// Create vertices
			const SDL_FPoint QUAD_SIZE = { 2.0f / window->GetCanvasWidth(), 2.0f / window->GetCanvasHeight() }; // We're in [-1..1] range
			const SDL_FPoint UPPER_LEFT = { -1.0f, 1.0f };

			for (int i = 0; i < NUM_VERTICES; i += 6)
			{
				const int quadIndex = i / 6;
				const int x = quadIndex % window->GetCanvasWidth();
				const int y = quadIndex / window->GetCanvasWidth();
				const SDL_FPoint QUAD_UPPER_LEFT = { UPPER_LEFT.x + QUAD_SIZE.x * x, UPPER_LEFT.y - QUAD_SIZE.y * y };

				const Uint8 v = screenBuffer[x][y] ?  255 : 0;

				// TODO switch to float4 and use A as an on/off?
				transferData[i] = {		QUAD_UPPER_LEFT.x,					QUAD_UPPER_LEFT.y,					0.0f,		v, v, v, 255 }; // upper left
				transferData[i + 1] = { QUAD_UPPER_LEFT.x + QUAD_SIZE.x,	QUAD_UPPER_LEFT.y,					0.0f,		v, v, v, 255 }; // upper right
				transferData[i + 2] = { QUAD_UPPER_LEFT.x,					QUAD_UPPER_LEFT.y - QUAD_SIZE.y,	0.0f,		v, v, v, 255 }; // lower left

				transferData[i + 3] = { QUAD_UPPER_LEFT.x + QUAD_SIZE.x,	QUAD_UPPER_LEFT.y,					0.0f,		v, v, v, 255 }; // upper right
				transferData[i + 4] = { QUAD_UPPER_LEFT.x,					QUAD_UPPER_LEFT.y - QUAD_SIZE.y,	0.0f,		v, v, v, 255 }; // lower left
				transferData[i + 5] = { QUAD_UPPER_LEFT.x + QUAD_SIZE.x,	QUAD_UPPER_LEFT.y - QUAD_SIZE.y,	0.0f,		v, v, v, 255 }; // lower right
			}

			SDL_UnmapGPUTransferBuffer(gpuDevice, transferBuffer);

			// Upload to vertex buffer
			SDL_GPUCommandBuffer* uploadCommandBuffer = SDL_AcquireGPUCommandBuffer(gpuDevice);
			SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCommandBuffer);
			
			SDL_GPUTransferBufferLocation source{};
			source.transfer_buffer = transferBuffer;
			source.offset = 0;

			SDL_GPUBufferRegion destination{};
			destination.buffer = sceneVertexBuffer;
			destination.offset = 0;
			destination.size = sizeof(PositionColorVertex) * NUM_VERTICES;

			SDL_UploadToGPUBuffer(copyPass, &source, &destination, false);
			SDL_EndGPUCopyPass(copyPass);
			SDL_SubmitGPUCommandBuffer(uploadCommandBuffer);
			SDL_ReleaseGPUTransferBuffer(gpuDevice, transferBuffer);

			// Draw
			SDL_DrawGPUPrimitives(renderPass, NUM_VERTICES, 1, 0, 0);
			SDL_EndGPURenderPass(renderPass);

			////////////////////////////// POST RENDER PASS //////////////////////////////

			SDL_GPUColorTargetInfo swapchainTargetInfo = { 0 };
			swapchainTargetInfo.texture = swapchainTexture;
			swapchainTargetInfo.clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
			swapchainTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
			swapchainTargetInfo.store_op = SDL_GPU_STOREOP_STORE;

			renderPass = SDL_BeginGPURenderPass(commandBuffer, &swapchainTargetInfo, 1, nullptr);
			SDL_BindGPUGraphicsPipeline(renderPass, postPipeline);
			
			SDL_GPUBufferBinding vertexBufferBinding{};
			vertexBufferBinding.buffer = postVertexBuffer;
			vertexBufferBinding.offset = 0;
			SDL_BindGPUVertexBuffers(renderPass, 0, &vertexBufferBinding, 1);

			SDL_GPUBufferBinding indexBufferBinding{};
			indexBufferBinding.buffer = postIndexBuffer;
			indexBufferBinding.offset = 0;
			SDL_BindGPUIndexBuffer(renderPass, &indexBufferBinding, SDL_GPU_INDEXELEMENTSIZE_16BIT);

			SDL_GPUTextureSamplerBinding textureSamplerBindings[1]{};
			textureSamplerBindings[0].texture = sceneTexture;
			textureSamplerBindings[0].sampler = sampler;
			SDL_BindGPUFragmentSamplers(renderPass, 0, textureSamplerBindings, 1);
			SDL_DrawGPUIndexedPrimitives(renderPass, 6, 1, 0, 0, 0);
			SDL_EndGPURenderPass(renderPass);

			//////////////////////////////////////////////////////////////////////////////

			// Set fragment shader uniform
			SDL_Point windowSize;
			SDL_GetWindowSize(window->GetSDLWindow(), &windowSize.x, &windowSize.y);
			ShaderUniform uni{ windowSize.x, windowSize.y };
			SDL_PushGPUFragmentUniformData(commandBuffer, 0, &uni, sizeof(ShaderUniform));

		}

		SDL_SubmitGPUCommandBuffer(commandBuffer);

		redraw = false;
	}

	nextRenderTime = SDL_GetTicks() + (1000.f / FRAMES_PER_SECOND);
}

void Renderer::Clear()
{
	// Clear screen
	//SDL_SetRenderDrawColor(sdlRenderer, 0, 0, 0, 255);
	//SDL_RenderClear(sdlRenderer);

	screenBuffer = vector<vector<bool>>(window->GetCanvasWidth(), vector<bool>(window->GetCanvasHeight(), false));
}

void Renderer::Display(uint8_t x, uint8_t y, uint8_t n, uint16_t I, vector<uint8_t>& memory, vector<uint8_t>& vars)
{
	x = x % window->GetCanvasWidth();
	const uint8_t originalX = x;
	y = y % window->GetCanvasHeight();

	vars[0xF] = 0;

	for (uint8_t row = 0; row < n && y < window->GetCanvasHeight(); row++, y++)
	{
		const uint8_t spriteData = memory[I + row];
		x = originalX;

		for (int col = 7; col >= 0 && x < window->GetCanvasWidth(); col--, x++)
		{
			bool currentPixel = screenBuffer[x][y];
			bool newPixel = (spriteData & (1 << col)); // Grab the relevant bit from spriteData

			if (newPixel && currentPixel)
				vars[0xF] = 1;
			
			newPixel ^= currentPixel;

			screenBuffer[x][y] = newPixel;
		}
	}

	redraw = true;
}

bool Renderer::OnWindowEvent(void* data, SDL_Event* event)
{
	if (event->type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED || event->type == SDL_EVENT_WINDOW_RESIZED)
	{
		Renderer* renderer = static_cast<Renderer*>(data);
		renderer->redraw = true;
	}

	return false;
}

bool Renderer::SetupDevice()
{
	gpuDevice = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV | SDL_GPU_SHADERFORMAT_DXIL | SDL_GPU_SHADERFORMAT_MSL, false, nullptr);

	if (gpuDevice == nullptr)
	{
		SDL_Log("Failed to create SDL_GPUDevice");
		return false;
	}

	if (!SDL_ClaimWindowForGPUDevice(gpuDevice, window->GetSDLWindow()))
	{
		SDL_Log("Failed to claim window for GPU");
		return false;
	}

	return true;
}

bool Renderer::SetupScenePipeline()
{
	// Setup shaders
	SDL_GPUShader* vertexShader = LoadShader(gpuDevice, "scene.vert", 0, 0, 0, 0);
	if (vertexShader == nullptr)
	{
		SDL_Log("Failed to create scene vertex shader.");
		return false;
	}	

	SDL_GPUShader* fragmentShader = LoadShader(gpuDevice, "scene.frag", 0, 0, 0, 0);
	if (fragmentShader == nullptr)
	{
		SDL_Log("Failed to create scene fragment shader.");
		return false;
	}

	// Setup vertex info so it matches PositionColorVertex
	SDL_GPUVertexBufferDescription vertexBufferDescriptions[1]{};
	vertexBufferDescriptions[0].slot = 0;
	vertexBufferDescriptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
	vertexBufferDescriptions[0].instance_step_rate = 0;
	vertexBufferDescriptions[0].pitch = sizeof(PositionColorVertex);

	SDL_GPUVertexAttribute vertexAttributes[2]{};

	// Vertex position
	vertexAttributes[0].buffer_slot = 0;
	vertexAttributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	vertexAttributes[0].location = 0;
	vertexAttributes[0].offset = 0;

	// Vertex color
	vertexAttributes[1].buffer_slot = 0;
	vertexAttributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_UBYTE4_NORM;
	vertexAttributes[1].location = 1;
	vertexAttributes[1].offset = sizeof(float) * 3; // Offset by the float3 of position

	SDL_GPUVertexInputState vertexInputState{};
	vertexInputState.num_vertex_buffers = 1;
	vertexInputState.vertex_buffer_descriptions = vertexBufferDescriptions;
	vertexInputState.num_vertex_attributes = 2;
	vertexInputState.vertex_attributes = vertexAttributes;

	// Set up target info
	SDL_GPUColorTargetDescription colorTargetDescriptions[1]{};
	colorTargetDescriptions[0].format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;

	SDL_GPUGraphicsPipelineTargetInfo targetInfo{};
	targetInfo.num_color_targets = 1;
	targetInfo.color_target_descriptions = colorTargetDescriptions;

	SDL_GPURasterizerState rasterizerState{};
	rasterizerState.cull_mode = SDL_GPU_CULLMODE_NONE;
	rasterizerState.fill_mode = SDL_GPU_FILLMODE_FILL;
	rasterizerState.front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE;

	// Set up actual graphics pipeline
	SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.vertex_shader = vertexShader;
	pipelineCreateInfo.fragment_shader = fragmentShader;
	pipelineCreateInfo.vertex_input_state = vertexInputState;
	pipelineCreateInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
	pipelineCreateInfo.target_info = targetInfo;
	pipelineCreateInfo.rasterizer_state = rasterizerState;

	// Create graphics pipeline
	scenePipeline = SDL_CreateGPUGraphicsPipeline(gpuDevice, &pipelineCreateInfo);
	if (scenePipeline == nullptr)
	{
		SDL_Log("Failed to create scene pipeline.");
		return false;
	}

	SDL_ReleaseGPUShader(gpuDevice, vertexShader);
	SDL_ReleaseGPUShader(gpuDevice, fragmentShader);

	return true;
}

bool Renderer::SetupPostPipeline()
{
	// Setup shaders
	SDL_GPUShader* vertexShader = LoadShader(gpuDevice, "post.vert", 0, 0, 0, 0);
	if (vertexShader == nullptr)
	{
		SDL_Log("Failed to create post vertex shader.");
		return false;
	}	

	SDL_GPUShader* fragmentShader = LoadShader(gpuDevice, "post.frag", 1, 1, 0, 0);
	if (fragmentShader == nullptr)
	{
		SDL_Log("Failed to create post fragment shader.");
		return false;
	}

	// Setup vertex info so it matches PositionTextureVertex
	SDL_GPUVertexBufferDescription vertexBufferDescriptions[1]{};
	vertexBufferDescriptions[0].slot = 0;
	vertexBufferDescriptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
	vertexBufferDescriptions[0].instance_step_rate = 0;
	vertexBufferDescriptions[0].pitch = sizeof(PositionTextureVertex);

	SDL_GPUVertexAttribute vertexAttributes[2]{};

	// Vertex position
	vertexAttributes[0].buffer_slot = 0;
	vertexAttributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
	vertexAttributes[0].location = 0;
	vertexAttributes[0].offset = 0;

	// Vertex UVs
	vertexAttributes[1].buffer_slot = 0;
	vertexAttributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
	vertexAttributes[1].location = 1;
	vertexAttributes[1].offset = sizeof(float) * 3; // Offset by the float3 of position

	SDL_GPUVertexInputState vertexInputState{};
	vertexInputState.num_vertex_buffers = 1;
	vertexInputState.vertex_buffer_descriptions = vertexBufferDescriptions;
	vertexInputState.num_vertex_attributes = 2;
	vertexInputState.vertex_attributes = vertexAttributes;

	// Set up target info
	SDL_GPUColorTargetBlendState colorTargetBlendState{};
	colorTargetBlendState.enable_blend = true;
	colorTargetBlendState.src_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
	colorTargetBlendState.dst_color_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	colorTargetBlendState.color_blend_op = SDL_GPU_BLENDOP_ADD;
	colorTargetBlendState.src_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE;
	colorTargetBlendState.dst_alpha_blendfactor = SDL_GPU_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
	colorTargetBlendState.alpha_blend_op = SDL_GPU_BLENDOP_ADD;

	SDL_GPUColorTargetDescription colorTargetDescriptions[1]{};
	colorTargetDescriptions[0].format = SDL_GetGPUSwapchainTextureFormat(gpuDevice, window->GetSDLWindow());
	colorTargetDescriptions[0].blend_state = colorTargetBlendState;

	SDL_GPUGraphicsPipelineTargetInfo targetInfo{};
	targetInfo.num_color_targets = 1;
	targetInfo.color_target_descriptions = colorTargetDescriptions;

	// Set up actual graphics pipeline
	SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo{};
	pipelineCreateInfo.vertex_shader = vertexShader;
	pipelineCreateInfo.fragment_shader = fragmentShader;
	pipelineCreateInfo.vertex_input_state = vertexInputState;
	pipelineCreateInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;
	pipelineCreateInfo.target_info = targetInfo;

	// Create graphics pipeline
	postPipeline = SDL_CreateGPUGraphicsPipeline(gpuDevice, &pipelineCreateInfo);
	if (postPipeline == nullptr)
	{
		SDL_Log("Failed to create post pipeline.");
		return false;
	}

	SDL_ReleaseGPUShader(gpuDevice, vertexShader);
	SDL_ReleaseGPUShader(gpuDevice, fragmentShader);

	return true;
}

SDL_GPUShader* Renderer::LoadShader(SDL_GPUDevice* device, const char* shaderFilename, Uint32 samplerCount, Uint32 uniformBufferCount, Uint32 storageBufferCount, Uint32 storageTextureCount)
{
	// Determine whether we're dealing with either a vertex or fragment shader
	SDL_GPUShaderStage stage;
	if (SDL_strstr(shaderFilename, ".vert"))
	{
		stage = SDL_GPU_SHADERSTAGE_VERTEX;
	}
	else if (SDL_strstr(shaderFilename, ".frag"))
	{
		stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
	}
	else
	{
		SDL_Log("Invalid shader stage: %s", shaderFilename);
		return nullptr;
	}

	// Set up right shader format
	char fullPath[256];
	SDL_GPUShaderFormat backendFormats = SDL_GetGPUShaderFormats(device);
	SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_INVALID;
	const char* entrypoint;

	//TODO differentiate path based on build type
	if (backendFormats & SDL_GPU_SHADERFORMAT_SPIRV) {
		SDL_snprintf(fullPath, sizeof(fullPath), "shaders/compiled/SPIRV/%s.spv", shaderFilename);
		format = SDL_GPU_SHADERFORMAT_SPIRV;
		entrypoint = "main";
	}
	else if (backendFormats & SDL_GPU_SHADERFORMAT_MSL) {
		SDL_snprintf(fullPath, sizeof(fullPath), "shaders/compiled/MSL/%s.msl", shaderFilename);
		format = SDL_GPU_SHADERFORMAT_MSL;
		entrypoint = "main0";
	}
	else if (backendFormats & SDL_GPU_SHADERFORMAT_DXIL) {
		SDL_snprintf(fullPath, sizeof(fullPath), "shaders/compiled/DXIL/%s.dxil", shaderFilename);
		format = SDL_GPU_SHADERFORMAT_DXIL;
		entrypoint = "main";
	}
	else 
	{
		SDL_Log("Unrecognized backend shader format: %s", fullPath);
		return nullptr;
	}

	// Load file
	size_t codeSize;
	void* code = SDL_LoadFile(fullPath, &codeSize);
	if (code == nullptr)
	{
		SDL_Log("Failed to load shader from disk! %s", fullPath);
		return nullptr;
	}

	// Create shader
	SDL_GPUShaderCreateInfo shaderInfo
	{
		.code_size = codeSize,
		.code = (Uint8*)code,
		.entrypoint = entrypoint,
		.format = format,
		.stage = stage,
		.num_samplers = samplerCount,
		.num_storage_textures = storageTextureCount,
		.num_storage_buffers = storageBufferCount,
		.num_uniform_buffers = uniformBufferCount
	};

	SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shaderInfo);
	if (shader == nullptr)
	{
		SDL_Log("Failed to create shader.");
		SDL_free(code);
		return nullptr;
	}

	SDL_free(code);

	return shader;
	
}

void Renderer::UpdateRenderScale() const
{
	//// Get our window's current width/height
	//int currentWidth;
	//int currentHeight;
	//SDL_GetWindowSize(window->GetSDLWindow(), &currentWidth, &currentHeight);

	//// Calculate the scale of canvas in relation to our window
	//const float scaleX = currentWidth / (float)window->GetCanvasWidth();
	//const float scaleY = currentHeight / (float)window->GetCanvasHeight();
	//
	//// Update render scale
	//SDL_SetRenderScale(sdlRenderer, scaleX, scaleY);
}

void Renderer::RenderBuffer()
{
	for (int y = 0; y < window->GetCanvasHeight(); y++)
	{
		for (int x = 0; x < window->GetCanvasWidth(); x++)
		{
			const int value = screenBuffer[x][y] ? 255 : 0;
			//SDL_SetRenderDrawColor(sdlRenderer, value, value, value, 255);
			//SDL_RenderPoint(sdlRenderer, x, y);
		}
	}
}

