#pragma once
#include <string>

struct Config {
    std::string windowTitle = "BluEngine";
    int windowWidth = 1280;
    int windowHeight = 720;
    bool vsync = true;
    bool debugMode = false;

    static Config& get();  // Singleton-style access
};
