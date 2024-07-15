#include "src/ui.h"
#include <filesystem>

int main()
{
    chess::Board board;

    board.init();

    std::filesystem::path path = std::filesystem::current_path();
    std::cout << "PATH: " << path << std::endl;

    std::filesystem::recursive_directory_iterator it(path);
    for(auto& entry : it){
        std::cout << entry.path() << std::endl;
    }

    ui::runWindow(board);
    return 0;
}