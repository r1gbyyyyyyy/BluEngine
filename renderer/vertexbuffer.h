#pragma once
#include <SDL3/SDL_gpu.h>
#include <vector>
#include "vertex.h"

class VertexBuffer {
public:
    VertexBuffer(SDL_GPUDevice* device, const std::vector<Vertex>& vertices);
    ~VertexBuffer();

    SDL_GPUBuffer* get() const { return buffer; }
    int getVertexCount() const { return count; }

private:
    SDL_GPUDevice* device;
    SDL_GPUBuffer* buffer = nullptr;
    int count = 0;
};
