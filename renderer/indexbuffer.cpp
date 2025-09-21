// renderer/indexbuffer.cpp
#include "indexbuffer.h"
#include <iostream>
#include <cstring>  // for memcpy

IndexBuffer::IndexBuffer(SDL_GPUDevice* device_, const std::vector<uint32_t>& indices)
    : device(device_), count(indices.size())
{
    if (!device || indices.empty()) {
        std::cerr << "IndexBuffer: invalid device or empty indices\n";
        return;
    }

    size_t bufferSize = indices.size() * sizeof(uint32_t);

    // Create GPU buffer with INDEX usage and COPY_DST
    SDL_GPUBufferCreateInfo bufInfo{};
    bufInfo.size = static_cast<Uint32>(bufferSize);
    bufInfo.usage = SDL_GPU_BUFFERUSAGE_INDEX;

    buffer = SDL_CreateGPUBuffer(device, &bufInfo);
    if (!buffer) {
        std::cerr << "SDL_CreateGPUBuffer(IndexBuffer) failed: " << SDL_GetError() << "\n";
        return;
    }

    // Create transfer buffer for staging
    SDL_GPUTransferBufferCreateInfo tbInfo{};
    tbInfo.size = static_cast<Uint32>(bufferSize);
    tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;

    SDL_GPUTransferBuffer* staging = SDL_CreateGPUTransferBuffer(device, &tbInfo);
    if (!staging) {
        std::cerr << "SDL_CreateGPUTransferBuffer failed: " << SDL_GetError() << "\n";
        // clean up GPU buffer
        SDL_ReleaseGPUBuffer(device, buffer);
        buffer = nullptr;
        return;
    }

    // Map to CPU
    void* mapped = SDL_MapGPUTransferBuffer(device, staging, /*cycle=*/false);
    if (!mapped) {
        std::cerr << "SDL_MapGPUTransferBuffer failed: " << SDL_GetError() << "\n";
        SDL_ReleaseGPUTransferBuffer(device, staging);
        SDL_ReleaseGPUBuffer(device, buffer);
        buffer = nullptr;
        return;
    }

    // Copy indices data
    std::memcpy(mapped, indices.data(), bufferSize);

    // Unmap
    SDL_UnmapGPUTransferBuffer(device, staging);

    // Copy to GPU buffer
    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
    if (!cmd) {
        std::cerr << "SDL_AcquireGPUCommandBuffer failed: " << SDL_GetError() << "\n";
        SDL_ReleaseGPUTransferBuffer(device, staging);
        return;
    }

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmd);
    if (!copyPass) {
        std::cerr << "SDL_BeginGPUCopyPass failed: " << SDL_GetError() << "\n";
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
    dstRegion.size = static_cast<Uint32>(bufferSize);

    // Upload: source is transfer location, destination is buffer region
    SDL_UploadToGPUBuffer(copyPass, &srcLoc, &dstRegion, /*cycle=*/false);

    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(cmd);

    // Release staging buffer
    SDL_ReleaseGPUTransferBuffer(device, staging);
}

IndexBuffer::~IndexBuffer() {
    if (buffer) {
        SDL_ReleaseGPUBuffer(device, buffer);
        buffer = nullptr;
    }
}
