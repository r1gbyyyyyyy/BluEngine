#pragma once
#include <SDL3/SDL_gpu.h>
#include <string>
#include <unordered_map>

class ShaderManager {
public:
    explicit ShaderManager(SDL_GPUDevice* device);
    ~ShaderManager();

    // Loads a shader from file and caches it by name
    SDL_GPUShader* loadShader(const std::string& name,
                              const std::string& path,
                              SDL_GPUShaderStage stage,
                              SDL_GPUShaderFormat format = SDL_GPU_SHADERFORMAT_SPIRV);

    // Get already loaded shader by name
    SDL_GPUShader* get(const std::string& name) const;

    // Free all shaders
    void clear();

private:
    SDL_GPUDevice* device;
    std::unordered_map<std::string, SDL_GPUShader*> shaders;
};
