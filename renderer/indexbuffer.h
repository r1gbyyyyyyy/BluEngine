#pragma once
#include <SDL3/SDL_gpu.h>
#include <vector>
#include <cstdint>

class IndexBuffer {
public:
    IndexBuffer(SDL_GPUDevice* device, const std::vector<uint32_t>& indices);
    ~IndexBuffer();

    SDL_GPUBuffer* get() const { return buffer; }
    size_t getCount() const { return count; }

private:
    SDL_GPUDevice* device = nullptr;
    SDL_GPUBuffer* buffer = nullptr;
    size_t count = 0;
};