#pragma once
#include <SDL3/SDL_gpu.h>
#include <string>
#include <unordered_map>

class PipelineManager {
public:
    explicit PipelineManager(SDL_GPUDevice* device);
    ~PipelineManager();

    SDL_GPUGraphicsPipeline* createGraphicsPipeline(
        const std::string& name,
        SDL_GPUShader* vertexShader,
        SDL_GPUShader* fragmentShader,
        SDL_GPUTextureFormat colorFormat);

    SDL_GPUGraphicsPipeline* getPipeline(const std::string& name) const;

private:
    SDL_GPUDevice* device;
    std::unordered_map<std::string, SDL_GPUGraphicsPipeline*> pipelines;
};
