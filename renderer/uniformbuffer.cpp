#include "uniformbuffer.h"
#include <iostream>
#include <cstring>

UniformBuffer::UniformBuffer(SDL_GPUDevice* device_, size_t size)
    : device(device_), buffer(nullptr), bufferSize(size)
{
    if (!device || size == 0) {
        std::cerr << "UniformBuffer: invalid device or size\n";
        return;
    }

    SDL_GPUBufferCreateInfo bufInfo{};
    bufInfo.size = static_cast<Uint32>(size);
    bufInfo.usage = SDL_GPU_BUFFERUSAGE_GRAPHICS_STORAGE_READ;
    // Note: storage read means read-only storage buffer in graphics stage. Docs require this. :contentReference[oaicite:4]{index=4}

    buffer = SDL_CreateGPUBuffer(device, &bufInfo);
    if (!buffer) {
        std::cerr << "UniformBuffer: SDL_CreateGPUBuffer failed: " << SDL_GetError() << "\n";
    }
}

UniformBuffer::~UniformBuffer() {
    if (buffer) {
        SDL_ReleaseGPUBuffer(device, buffer);
        buffer = nullptr;
    }
}

void UniformBuffer::update(const void* data, size_t size) {
    if (!buffer || !device || size > bufferSize) {
        std::cerr << "UniformBuffer: invalid parameters in update\n";
        return;
    }

    // Create transfer buffer
    SDL_GPUTransferBufferCreateInfo tbInfo{};
    tbInfo.size = static_cast<Uint32>(size);
    tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;  // upload staging buffer

    SDL_GPUTransferBuffer* staging = SDL_CreateGPUTransferBuffer(device, &tbInfo);
    if (!staging) {
        std::cerr << "UniformBuffer: SDL_CreateGPUTransferBuffer failed: " << SDL_GetError() << "\n";
        return;
    }

    // Map staging
    void* mapped = SDL_MapGPUTransferBuffer(device, staging, /*cycle=*/false);
    if (!mapped) {
        std::cerr << "UniformBuffer: SDL_MapGPUTransferBuffer failed: " << SDL_GetError() << "\n";
        SDL_ReleaseGPUTransferBuffer(device, staging);
        return;
    }

    std::memcpy(mapped, data, size);
    SDL_UnmapGPUTransferBuffer(device, staging);

    // Copy pass
    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
    if (!cmd) {
        std::cerr << "UniformBuffer: SDL_AcquireGPUCommandBuffer failed: " << SDL_GetError() << "\n";
        SDL_ReleaseGPUTransferBuffer(device, staging);
        return;
    }

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmd);
    if (!copyPass) {
        std::cerr << "UniformBuffer: SDL_BeginGPUCopyPass failed: " << SDL_GetError() << "\n";
        SDL_SubmitGPUCommandBuffer(cmd);
        SDL_ReleaseGPUTransferBuffer(device, staging);
        return;
    }

    SDL_GPUTransferBufferLocation srcLoc{};
    srcLoc.transfer_buffer = staging;
    srcLoc.offset = 0;

    SDL_GPUBufferRegion dstRegion{};
    dstRegion.buffer = buffer;
    dstRegion.offset = 0;
    dstRegion.size = static_cast<Uint32>(size);

    SDL_UploadToGPUBuffer(copyPass, &srcLoc, &dstRegion, /*cycle=*/false);

    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(cmd);

    SDL_ReleaseGPUTransferBuffer(device, staging);
}
