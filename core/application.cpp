#include "config.h"
#include "application.h"
#include "etime.h"
#include <iostream>

Application::Application(const std::string& title, int width, int height) {
    Config& cfg = Config::get();

    std::string useTitle = title.empty() ? cfg.windowTitle : title;
    if (width <= 0) width = cfg.windowWidth;
    if (height <= 0) height = cfg.windowHeight;

    if (!initSDL(title, width, height)) {
        std::cerr << "Failed to initialize SDL Application\n";
    }

    renderer = new Renderer(device, window);
    renderer->setClearColor(0.1f, 0.1f, 0.3f, 1.0f); // bluish
}

Application::~Application() {
    shutdownSDL();
}

bool Application::initSDL(const std::string& title, int width, int height) {
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS)) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow(title.c_str(),
                              width, height,
                              SDL_WINDOW_RESIZABLE);
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    // Tell SDL what shader format(s) we can supply
    SDL_GPUShaderFormat format_flags = SDL_GPU_SHADERFORMAT_SPIRV;

    device = SDL_CreateGPUDevice(format_flags, /*debug_mode=*/false, /*name=*/nullptr);
    if (!device) {
        std::cerr << "SDL_CreateGPUDevice Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    if (!SDL_ClaimWindowForGPUDevice(device, window)) {
        std::cerr << "SDL_ClaimWindowForGPUDevice Error: " << SDL_GetError() << std::endl;
        SDL_DestroyGPUDevice(device);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    running = true;
    return true;
}

void Application::mainLoop() {
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
        Time::update();
        // for debugging, but it may spam it
        // so yeah, just don't use it for now
        // std::cout << "FPS: " << Time::fps() << "\n";
        // * This is where rendering code will go now
        if (renderer) {
            float time = SDL_GetTicks() / 1000.0f; // seconds
            renderer->UpdateUniform(time);
            renderer->beginFrame();
            // later you’ll add renderer->drawStuff()
            renderer->endFrame();
        }
    }
}

void Application::shutdownSDL() {
    if (device) {
        SDL_DestroyGPUDevice(device);
        device = nullptr;
    }

    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }

    SDL_Quit();
}