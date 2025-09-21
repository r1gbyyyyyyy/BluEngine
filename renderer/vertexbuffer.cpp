#include "vertexbuffer.h"
#include <iostream>

VertexBuffer::VertexBuffer(SDL_GPUDevice* device_, const std::vector<Vertex>& vertices)
    : device(device_), count(static_cast<int>(vertices.size()))
{
    if (!device || vertices.empty()) return;

    // Step 1: create a GPU buffer for vertices
    SDL_GPUBufferCreateInfo bufferInfo{};
    bufferInfo.size = sizeof(Vertex) * vertices.size();
    bufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;

    buffer = SDL_CreateGPUBuffer(device, &bufferInfo);
    if (!buffer) {
        std::cerr << "SDL_CreateGPUBuffer failed: " << SDL_GetError() << "\n";
        return;
    }

    // Step 2: create a transfer buffer for uploading data
    SDL_GPUTransferBufferCreateInfo tbInfo{};
    tbInfo.size = bufferInfo.size;
    tbInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;

    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(device, &tbInfo);
    if (!transferBuffer) {
        std::cerr << "SDL_CreateGPUTransferBuffer failed: " << SDL_GetError() << "\n";
        SDL_ReleaseGPUBuffer(device, buffer);
        buffer = nullptr;
        return;
    }

    // Map transfer buffer and fill data
    void* mapped = SDL_MapGPUTransferBuffer(device, transferBuffer, false);
    if (!mapped) {
        std::cerr << "SDL_MapGPUTransferBuffer failed: " << SDL_GetError() << "\n";
        SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
        SDL_ReleaseGPUBuffer(device, buffer);
        buffer = nullptr;
        return;
    }

    // Copy vertex data into mapped memory
    memcpy(mapped, vertices.data(), bufferInfo.size);

    // Unmap
    SDL_UnmapGPUTransferBuffer(device, transferBuffer);

    // Step 3: Begin a copy pass to upload from transfer buffer to GPU buffer
    SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
    if (!cmd) {
        std::cerr << "SDL_AcquireGPUCommandBuffer failed: " << SDL_GetError() << "\n";
        SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
        return;
    }

    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmd);
    if (!copyPass) {
        std::cerr << "SDL_BeginGPUCopyPass failed: " << SDL_GetError() << "\n";
        SDL_SubmitGPUCommandBuffer(cmd);
        SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
        return;
    }

    // Set up source location and destination region
    SDL_GPUTransferBufferLocation srcLoc{};
    srcLoc.transfer_buffer = transferBuffer;
    srcLoc.offset = 0;

    SDL_GPUBufferRegion dstRegion{};
    dstRegion.buffer = buffer;
    dstRegion.offset = 0;
    dstRegion.size = bufferInfo.size;

    // Upload
    SDL_UploadToGPUBuffer(copyPass, &srcLoc, &dstRegion, /*cycle=*/false);

    // End copy pass
    SDL_EndGPUCopyPass(copyPass);

    // Submit command buffer
    SDL_SubmitGPUCommandBuffer(cmd);

    // Clean up transfer buffer — we won't need it anymore
    SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
}

VertexBuffer::~VertexBuffer() {
    if (buffer) {
        SDL_ReleaseGPUBuffer(device, buffer);
        buffer = nullptr;
    }
}
