#include <cengine/cengine.h>

int main(int argc, char** argv)
{
    global_settings.base_path = std::filesystem::path(argv[0]).parent_path();

    uci::UCI uci;
    uci.loop();
    // uci.sendCommand("isready");
    // uci.sendCommand("ucinewgame");
    // uci.sendCommand("position fen r6r/1b2k1bq/8/8/7B/8/8/R3K2R b KQ - 3 2 moves h7h4");
    // uci.sendCommand("go movetime 1000");
    
    // std::this_thread::sleep_for(std::chrono::seconds(1));
    // uci.sendCommand("stop");
    // uci.sendCommand("position fen r6r/1b2k1bq/8/8/7B/8/8/R3K2R b KQ - 3 2 moves h7h4 h1h4");
    // uci.sendCommand("go movetime 1000");
    // std::this_thread::sleep_for(std::chrono::seconds(1));
    // uci.sendCommand("stop");
    // uci.sendCommand("quit");
    return 0;
}