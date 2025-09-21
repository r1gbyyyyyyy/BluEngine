#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <array>
#include "shadermanager.h"
#include "pipelinemanager.h"
#include "vertexbuffer.h"
#include "indexbuffer.h"
#include "uniformbuffer.h"

struct UniformData {
    float mvp[16];
};

class Renderer {
public:
    Renderer(SDL_GPUDevice* device, SDL_Window* window);
    ~Renderer();

    void beginFrame();
    void endFrame();
    void UpdateUniform(float time);
    void setClearColor(float r, float g, float b, float a);

private:

    SDL_GPUDevice* device = nullptr;
    SDL_Window* window = nullptr;
    ShaderManager* shaderMgr = nullptr;
    PipelineManager* pipelineMgr = nullptr;
    VertexBuffer* triangleVBO = nullptr;
    IndexBuffer* triangleIBO = nullptr;
    SDL_GPUBuffer* uniformBuffer = nullptr;
    SDL_GPUTransferBuffer* transferBuffer = nullptr;
    SDL_GPUGraphicsPipeline* pipeline = nullptr;
    std::array<float, 4> clearColor {0.1f, 0.1f, 0.1f, 1.0f}; // default dark gray
    UniformData currentUniform{};  // stores MVP for the current frame
};
