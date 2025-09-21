#include "shadermanager.h"
#include <SDL3/SDL.h>
#include <fstream>
#include <vector>
#include <iostream>

ShaderManager::ShaderManager(SDL_GPUDevice* device_) : device(device_) {}

ShaderManager::~ShaderManager() {
    clear();
}

SDL_GPUShader* ShaderManager::loadShader(const std::string& name,
                                         const std::string& path,
                                         SDL_GPUShaderStage stage,
                                         SDL_GPUShaderFormat format) {
    // If already loaded, return it
    auto it = shaders.find(name);
    if (it != shaders.end()) {
        return it->second;
    }

    // Load file into memory
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        std::cerr << "Failed to open shader file: " << path << "\n";
        return nullptr;
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        std::cerr << "Failed to read shader file: " << path << "\n";
        return nullptr;
    }

    // Create shader descriptor
    SDL_GPUShaderCreateInfo info = {};
    info.stage = stage;
    info.format = format;
    info.code_size = static_cast<Uint32>(buffer.size());
    info.code = buffer.data();

    SDL_GPUShader* shader = SDL_CreateGPUShader(device, &info);
    if (!shader) {
        std::cerr << "SDL_CreateGPUShader failed: " << SDL_GetError() << "\n";
        return nullptr;
    }

    shaders[name] = shader;
    return shader;
}

SDL_GPUShader* ShaderManager::get(const std::string& name) const {
    auto it = shaders.find(name);
    if (it != shaders.end()) {
        return it->second;
    }
    return nullptr;
}

void ShaderManager::clear() {
    for (auto& [name, shader] : shaders) {
        SDL_ReleaseGPUShader(device, shader);
    }
    shaders.clear();
}
