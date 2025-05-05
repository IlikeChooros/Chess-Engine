#include <ui/base_manager.hpp>


UI_NAMESPACE_BEGIN

// CHESS MANAGER

/**
 * @brief Adds --hash, --logfile, --threads options to the parser
 */
void ChessManager::M_add_base_engine_options()
{
    m_parser.addArg("--hash", "16", 
        chess::ArgParser::intValidator, 
            "Hash size in MB (default: 16)");
    m_parser.addArg("--logfile", "", 
        chess::ArgParser::stringValidator, 
            "Log file path");
}

void ChessManager::M_process_engine_options(chess::ArgParser::arg_map_t& args)
{

}

/**
 * @brief Adds --limits and --fen options to the parser
 */
void ChessManager::M_add_base_game_options()
{
    m_parser.addArg("--limits", "",
        [](std::string limits) { 
            std::istringstream iss(limits);
            try {
                uci::UCI::parseGoOptions(iss, false, false);
                return true;
            }
            catch(...) {}
            return false;
         }, 
            "Engine constraints (e.g. \"movetime 1000 depth 5\")");

    m_parser.addArg("--fen", "", 
        [](std::string fen) { return chess::Board().loadFen(fen); }, 
            "FEN string or 'startpos', support moves as well");
}

/**
 * @brief Process the input parameters (FEN, engine constraints), if one of them is not set,
 * ask the user for input
 */
void ChessManager::M_process_game_options(chess::ArgParser::arg_map_t& args)
{
    using namespace chess;

    // Check if the user specified given parameters
    if (!ArgParser::exists("--fen", args))
    {
        // Use a lambda function to validate the FEN string
        M_process_input(
            "Enter the FEN string (or 'startpos' for the initial position): ", 
            "Invalid FEN string:",
            [this](std::string fen) {
                if (!m_engine.setPosition(fen))
                    throw std::runtime_error(fen);
            }
        );
    } else {
        m_engine.setPosition(args["--fen"]);
    }

    // Check if limits were specified
    if (!ArgParser::exists("--limits", args))
    {
        M_process_input(
            "Enter the constraints: ",
            "Invalid constraints:",
            [this](std::string constraints) {
                // parseGoOptions will throw an exception if the input is invalid
                std::istringstream iss(constraints);
                m_options = uci::UCI::parseGoOptions(iss, false, false);
            }
        );
    } else {
        std::istringstream iss(args["--limits"]);
        m_options = uci::UCI::parseGoOptions(iss, false, false);
    }
}

// BASE MANAGER

/**
 * @brief Initialize the engine with base position
 */
void BaseManager::M_init()
{
    m_engine.setPosition();

    // Add the default argument
    m_parser.addArg("--ui", "true", 
        chess::ArgParser::defaultValidator, 
            "Enable the UI (default: true)");
}

/**
 * @brief Setup the game loop with the given parameters
 */
void BaseManager::M_loop_setup(int argc, char** argv)
{
    M_init();
    M_process_param_input(argc, argv);
    M_render();
}

/**
 * @brief Read from standard input
 */
std::string BaseManager::M_read_input()
{
    std::string input;
    std::getline(std::cin, input);
    return input;
}

/**
 * @brief Process the standard input, with messages
 * @param prompt The prompt to display, e.g. "Enter your move: "
 * @param validator The function to validate the input, should throw an exception on error
 * @param error_msg The error message to display on invalid input
 */
void BaseManager::M_process_input(
    const char* prompt, 
    const char* error_msg,
    callback_t validator 
)
{
    bool valid = true;
    do 
    {
        valid = true;
        print("\r", Ansi::BOLD, prompt);
        flush();

        auto input = M_read_input();
        if (input == "exit" || input == "quit" || input == "q")
        {
            std::cout << "Exiting...\n"; // Ensure newline before exit message
            exit(0);
        }

        // Call the validator function
        try {
            validator(input);
        }
        // Catch the error on invalid input, and display the error message
        catch(const std::exception& e) {
            valid = false;
            print_error(error_msg, e.what());
            // Go back up one line to overwrite the prompt
            move_cursor_up();
        }
    } while(!valid);

    move_cursor_up(1, true); // Clear the invalid input line
}

UI_NAMESPACE_END