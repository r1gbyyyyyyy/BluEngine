#include "config.h"

// Static global instance
Config& Config::get() {
    static Config instance;
    return instance;
}
