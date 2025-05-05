#include <ui/game_manager.hpp>

UI_NAMESPACE_BEGIN

void GameManager::loop(int argc, char** argv)
{
    M_loop_setup(argc, argv);
    clear();

    // Run the manager
    if (m_manager)
        m_manager->loop(argc, argv);
    else
        throw std::runtime_error("No manager selected");
}

/**
 * @brief Initialize the chess library, turn off logger
 */
void GameManager::M_init()
{
    chess::init();
    glogger.setPrint(false);
    glogger.setLog(false);

    BaseManager::M_init();

    m_parser.addArg("--singleplayer", "",
        chess::ArgParser::booleanValidator,
        "Play against the engine");

    m_parser.addArg("--analysis", "",
        chess::ArgParser::booleanValidator,
        "Analyze the position with the engine");
}

/**
 * @brief Process the command line arguments
 */
chess::ArgParser::arg_map_t GameManager::M_process_arguments(int argc, char** argv)
{
    m_parser.setArgs(argc, argv);

    m_parser.addArg("--singleplayer", "",
        chess::ArgParser::booleanValidator,
        "Play against the engine");

    m_parser.addArg("--analysis", "",
        chess::ArgParser::booleanValidator,
        "Analyze the position with the engine");

    return m_parser.parse();
}

/**
 * @brief Process the input parameters (FEN, side, engine constraints)
 */
void GameManager::M_process_param_input(int argc, char** argv)
{
    using namespace chess;

    // Process the command line arguments
    auto args = M_process_arguments(argc, argv);

    // Print out the version
    std::cout << "CEngine UCI ver " << global_settings.version << " with UI\n";

    // Check if the user specified given parameters
    if (ArgParser::exists("--singleplayer", args))
        m_manager = std::make_unique<SinglePlayer>();
    else if (ArgParser::exists("--analysis", args))
        m_manager = std::make_unique<Analysis>();
    else 
    {
        // Ask the user for the mode
        M_process_input(
            "Select the mode (singleplayer/analysis (s/a)): ",
            "Invalid mode",
            [this](std::string input) {
                if (input == "a" || input == "analysis")
                    m_manager = std::make_unique<Analysis>();
                else if (input == "s" || input == "singleplayer")
                    m_manager = std::make_unique<SinglePlayer>();
                else
                    throw std::runtime_error("please select s or a");
            }
        );
    }
}

UI_NAMESPACE_END