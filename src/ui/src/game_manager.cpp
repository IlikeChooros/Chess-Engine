#include <ui/game_manager.hpp>

UI_NAMESPACE_BEGIN

void GameManager::loop(int argc, char** argv)
{
    // Initialize the UI
    M_init();
    m_parser.setArgs(argc, argv);
    auto args = m_parser.parse();
    M_process_param_input(args);
    clear();

    // Run the manager
    if (m_manager)
    {
        m_manager->loop(args);
    }
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

    m_parser.add_argument({"--ui"}, {
        .required = true,
        .help_message = "Enable the UI (default: false)",
        .default_value = "",
        .type = chess::ArgParser::BOOL,
        .validator = chess::ArgParser::booleanValidator,
        .action = chess::ArgParser::storeTrue
    });

    m_parser.add_argument({"--singleplayer", "-s"}, {
        .required = false,
        .help_message = "Play against the engine",
        .default_value = "",
        .type = chess::ArgParser::BOOL,
        .validator = chess::ArgParser::booleanValidator,
        .action = chess::ArgParser::storeTrue
    });

    m_parser.add_argument({"--analysis", "-a"}, {
        .required = false,
        .help_message = "Analyze the position with the engine",
        .default_value = "",
        .type = chess::ArgParser::BOOL,
        .validator = chess::ArgParser::booleanValidator,
        .action = chess::ArgParser::storeTrue
    });

    ChessManager::add_args(m_parser); // add base engine options (hash, logfile, limits, fen)
    // Analysis::add_args(m_parser); 
    SinglePlayer::add_args(m_parser); // add (--side) option
}

/**
 * @brief Process the input parameters (FEN, side, engine constraints)
 */
void GameManager::M_process_param_input(arg_map_t& args)
{
    using namespace chess;

    // Print out the version
    std::cout << "CEngine UCI ver " << global_settings.version << " with UI\n";

    // Check if the user specified given parameters
    if (args.exists("--singleplayer"))
        m_manager = std::make_unique<SinglePlayer>();
    else if (args.exists("--analysis"))
        m_manager = std::make_unique<Analysis>();
    else 
    {
        // Ask the user for the mode
        M_process_input(
            "Select the mode (singleplayer/analysis, s/a): ",
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