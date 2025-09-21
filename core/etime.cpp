#include "etime.h"

void Time::update() {
    using namespace std::chrono;

    auto now = high_resolution_clock::now();

    if (!initialized) {
        startTime = now;
        lastFrame = now;
        initialized = true;
        delta = 0.0f;
        total = 0.0;
        frameRate = 0.0f;
        return;
    }

    duration<float> frameDelta = now - lastFrame;
    delta = frameDelta.count();

    duration<double> sinceStart = now - startTime;
    total = sinceStart.count();

    // Avoid divide by zero
    frameRate = (delta > 0.0f) ? (1.0f / delta) : 0.0f;

    lastFrame = now;
}

float Time::deltaTime() {
    return delta;
}

double Time::totalTime() {
    return total;
}

float Time::fps() {
    return frameRate;
}
