#include <cengine/settings.h>

Settings global_settings = {
    .debug_level = DEBUG_LEVEL,
    .debug_performance = DEBUG_PERFORMANCE,
    .base_path = std::filesystem::current_path()
}; 