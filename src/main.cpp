#include <cengine/cengine.h>

int main(int argc, char** argv)
{
    global_settings.base_path = std::filesystem::path(argv[0]).parent_path();

    uci::UCI uci;
    uci.loop();
    
    return 0;
}