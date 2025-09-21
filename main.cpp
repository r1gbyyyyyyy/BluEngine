#include <iostream>
#include "core/application.h"
#include "core/config.h"

int main() {
    Config& cfg = Config::get();
    cfg.windowTitle = "BluEngine Application";
    cfg.windowWidth = 1600;
    cfg.windowHeight = 900;
    cfg.debugMode = true;

    Application app(cfg.windowTitle, cfg.windowWidth, cfg.windowHeight);
    app.mainLoop();
    return 0;
}