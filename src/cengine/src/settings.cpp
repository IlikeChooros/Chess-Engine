#include <cengine/settings.h>

Settings global_settings = {
    DEBUG_LEVEL,
    DEBUG_PERFORMANCE,
    std::filesystem::current_path(),
    "3.0-hash_on_move-faster_move_gen"
}; 