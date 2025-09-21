#include "pipelinemanager.h"
#include "vertex.h"
#include <iostream>
#include <cstddef>  // for offsetof

PipelineManager::PipelineManager(SDL_GPUDevice* device_)
    : device(device_) {}

PipelineManager::~PipelineManager() {
    for (auto& kv : pipelines) {
        if (kv.second) {
            SDL_ReleaseGPUGraphicsPipeline(device, kv.second);
        }
    }
    pipelines.clear();
}

SDL_GPUGraphicsPipeline* PipelineManager::createGraphicsPipeline(
    const std::string& name,
    SDL_GPUShader* vertexShader,
    SDL_GPUShader* fragmentShader,
    SDL_GPUTextureFormat colorFormat)
{
    if (!device || !vertexShader || !fragmentShader) {
        std::cerr << "PipelineManager: invalid device or shaders for pipeline '" << name << "'\n";
        return nullptr;
    }

    // Setup vertex buffer description
    SDL_GPUVertexBufferDescription vbDesc{};
    vbDesc.slot = 0;
    vbDesc.input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vbDesc.instance_step_rate = 0;
    vbDesc.pitch = sizeof(Vertex);

    // Setup vertex attributes
    SDL_GPUVertexAttribute attribs[2]{};

    attribs[0].buffer_slot = 0;
    attribs[0].location = 0;
    attribs[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    attribs[0].offset = offsetof(Vertex, position);

    attribs[1].buffer_slot = 0;
    attribs[1].location = 1;
    attribs[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    attribs[1].offset = offsetof(Vertex, color);

    SDL_GPUVertexInputState inputState{};
    inputState.vertex_buffer_descriptions = &vbDesc;
    inputState.num_vertex_buffers = 1;
    inputState.vertex_attributes = attribs;
    inputState.num_vertex_attributes = 2;

    SDL_GPUGraphicsPipelineCreateInfo pipeInfo{};
    pipeInfo.vertex_shader = vertexShader;
    pipeInfo.fragment_shader = fragmentShader;
    pipeInfo.vertex_input_state = inputState;
    pipeInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

    // The target info: what color formats + blend settings
    SDL_GPUColorTargetDescription colorDesc{};
    colorDesc.format = colorFormat;

    SDL_GPUGraphicsPipelineTargetInfo targetInfo{};
    targetInfo.color_target_descriptions = &colorDesc;
    targetInfo.num_color_targets = 1;

    pipeInfo.target_info = targetInfo;

    SDL_GPUGraphicsPipeline* pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipeInfo);
    if (!pipeline) {
        std::cerr << "SDL_CreateGPUGraphicsPipeline failed for '" << name
                  << "': " << SDL_GetError() << "\n";
        return nullptr;
    }

    pipelines[name] = pipeline;
    return pipeline;
}

SDL_GPUGraphicsPipeline* PipelineManager::getPipeline(const std::string& name) const {
    auto it = pipelines.find(name);
    if (it != pipelines.end()) {
        return it->second;
    }
    return nullptr;
}
