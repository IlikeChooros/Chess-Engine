#include <cengine/cengine.h>
#include <ui/ui.hpp>

int main(int argc, char** argv)
{
    global_settings.base_path = std::filesystem::path(argv[0]).parent_path();

    if (argc >= 2 && std::string(argv[1]) == "--ui")
    {
        ui::GameManager man;
        man.loop(argc, argv);
    }
    else
    {
        uci::UCI uci;
        uci.loop(argc, argv);
    }

    return 0;
}