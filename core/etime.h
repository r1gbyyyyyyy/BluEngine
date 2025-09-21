#pragma once
#include <chrono>

class Time {
public:
    // Called once per frame
    static void update();

    // Getters
    static float deltaTime();     // Seconds between last frame and this one
    static double totalTime();    // Total time since start (seconds)
    static float fps();           // Frames per second

private:
    static inline std::chrono::high_resolution_clock::time_point lastFrame{};
    static inline std::chrono::high_resolution_clock::time_point startTime{};
    static inline float delta = 0.0f;
    static inline double total = 0.0;
    static inline float frameRate = 0.0f;
    static inline bool initialized = false;
};
