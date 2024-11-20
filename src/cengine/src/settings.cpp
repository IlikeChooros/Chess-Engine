#include <cengine/settings.h>

Settings global_settings = {
    DEBUG_LEVEL,
    DEBUG_PERFORMANCE,
    std::filesystem::current_path(),
    "0.0"
}; 