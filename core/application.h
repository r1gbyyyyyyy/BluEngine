#pragma once
#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>
#include <string>
#include "../renderer/renderer.h"
#include "etime.h"
#include "config.h"

class Application {
public:
    Application(const std::string& title, int width, int height);
    ~Application();

    void mainLoop();
private:
    bool initSDL(const std::string& title, int width, int height);
    void shutdownSDL();

    SDL_Window* window = nullptr;
    SDL_GPUDevice* device = nullptr;
    Renderer* renderer = nullptr;
    bool running = false;
};
