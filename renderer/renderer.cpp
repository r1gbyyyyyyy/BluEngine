#include "renderer.h"
#include <iostream>
#include <cmath>
#include <cstring>

Renderer::Renderer(SDL_GPUDevice* device_, SDL_Window* window_)
    : device(device_), window(window_) {
    if (!device || !window) {
        std::cerr << "Renderer initialization failed: invalid device or window\n";
    }

    shaderMgr   = new ShaderManager(device);
    pipelineMgr = new PipelineManager(device);

    // Query swapchain format
    SDL_GPUTextureFormat swapchainFormat = SDL_GetGPUSwapchainTextureFormat(device, window);

    auto vert = shaderMgr->loadShader("basic_vert", "shaders/basic.vert.spv", SDL_GPU_SHADERSTAGE_VERTEX);
    auto frag = shaderMgr->loadShader("basic_frag", "shaders/basic.frag.spv", SDL_GPU_SHADERSTAGE_FRAGMENT);

    pipeline = pipelineMgr->createGraphicsPipeline("basic", vert, frag, swapchainFormat);
    if (!pipeline) {
        std::cerr << "Pipeline creation failed: " << SDL_GetError() << "\n";
    }

    std::vector<Vertex> verts = {
        {{-0.5f, -0.5f, 0.0f}, {1,0,0}}, // bottom-left
        {{ 0.5f, -0.5f, 0.0f}, {0,1,0}}, // bottom-right
        {{ 0.0f,  0.5f, 0.0f}, {0,0,1}}, // top
    };
    triangleVBO = new VertexBuffer(device, verts);

    // optional UBOs (not actually used since we push directly)
    SDL_GPUBufferCreateInfo uboInfo{};
    uboInfo.size  = sizeof(UniformData);
    uboInfo.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
    uniformBuffer = SDL_CreateGPUBuffer(device, &uboInfo);

    SDL_GPUTransferBufferCreateInfo tboInfo{};
    tboInfo.size   = sizeof(UniformData);
    tboInfo.usage  = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transferBuffer = SDL_CreateGPUTransferBuffer(device, &tboInfo);
}

Renderer::~Renderer() {
    if (uniformBuffer) SDL_ReleaseGPUBuffer(device, uniformBuffer);
    if (transferBuffer) SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
    delete triangleVBO;
    delete pipelineMgr;
    delete shaderMgr;
}

void Renderer::beginFrame() {
    if (!device || !window) return;

    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
    if (!cmd) {
        std::cerr << "SDL_AcquireGPUCommandBuffer failed: " << SDL_GetError() << "\n";
        return;
    }

    SDL_GPUTexture* swapTex = nullptr;
    Uint32 texW = 0, texH = 0;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmd, window, &swapTex, &texW, &texH)) {
        std::cerr << "SDL_WaitAndAcquireGPUSwapchainTexture failed: " << SDL_GetError() << "\n";
        SDL_SubmitGPUCommandBuffer(cmd);
        return;
    }

    SDL_GPUColorTargetInfo colorTarget{};
    colorTarget.texture     = swapTex;
    colorTarget.clear_color = { clearColor[0], clearColor[1], clearColor[2], clearColor[3] };
    colorTarget.load_op     = SDL_GPU_LOADOP_CLEAR;
    colorTarget.store_op    = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmd, &colorTarget, 1, nullptr);
    if (!pass) {
        std::cerr << "SDL_BeginGPURenderPass failed: " << SDL_GetError() << "\n";
        SDL_SubmitGPUCommandBuffer(cmd);
        return;
    }

    if (pipeline && triangleVBO) {
        SDL_BindGPUGraphicsPipeline(pass, pipeline);

        SDL_GPUBufferBinding vb{};
        vb.buffer = triangleVBO->get();
        vb.offset = 0;
        SDL_BindGPUVertexBuffers(pass, 0, &vb, 1);

        // Push uniform for this draw
        SDL_PushGPUVertexUniformData(cmd, 1, &currentUniform, sizeof(currentUniform));

        SDL_DrawGPUPrimitives(pass, 3, 1, 0, 0);
    }

    SDL_EndGPURenderPass(pass);
    SDL_SubmitGPUCommandBuffer(cmd);
}

void Renderer::endFrame() {
    // Present handled by SDL internally
}

void Renderer::setClearColor(float r, float g, float b, float a) {
    clearColor = {r, g, b, a};
}

void Renderer::UpdateUniform(float time) {
    float angle = time;
    float c = cosf(angle);
    float s = sinf(angle);

    // column-major rotation Z matrix (GLSL expects column-major)
    float mvp[16] = {
        c,  -s, 0.f, 0.f,
        s,   c, 0.f, 0.f,
        0,  0,  1.f, 0.f,
        0,  0,  0.f, 1.f
    };

    std::memcpy(currentUniform.mvp, mvp, sizeof(mvp));
}
