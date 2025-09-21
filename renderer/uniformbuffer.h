#pragma once
#include <SDL3/SDL_gpu.h>
#include <cstddef>

class UniformBuffer {
public:
    UniformBuffer(SDL_GPUDevice* device, size_t size);
    ~UniformBuffer();

    // Update data (replaces previous contents)
    void update(const void* data, size_t size);

    SDL_GPUBuffer* getBuffer() const { return buffer; }

private:
    SDL_GPUDevice* device;
    SDL_GPUBuffer* buffer;
    size_t bufferSize;
};
