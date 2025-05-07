#include <ui/base_manager.hpp>


UI_NAMESPACE_BEGIN

// CHESS MANAGER

/**
 * @brief Adds --hash, --logfile, --threads options to the parser
 */
void ChessManager::M_add_base_engine_options(arg_parser_t& parser)
{
    parser.add_argument({"--logfile"}, {
        .required = false,
        .help_message = "Set the log file",
        .default_value = "",
        .type = arg_parser_t::STRING,
        .validator = chess::ArgParser::stringValidator,
        .action = [](chess::ArgParser::arg_value_t& value, std::string sval) {
            glogger.setLogFile(sval);
            // glogger.setLog(true);
        }
    });

    parser.add_argument({"--hash"}, {
        .required = false,
        .help_message = "Set the hash size (in MB)",
        .default_value = "16",
        .type = arg_parser_t::INT,
        .validator = chess::ArgParser::intValidator,
        .action = [](chess::ArgParser::arg_value_t& value, std::string sval) {
            value = std::stoi(sval);
            if (std::get<int>(value) < 1)
                throw std::invalid_argument("Hash size must be greater than 0");
        }
    });
}

void ChessManager::M_process_engine_options(chess::ArgParser::arg_map_t& args)
{
    m_engine.setHashSize(args.get<int>("--hash"));
}

/**
 * @brief Adds --limits and --fen options to the parser
 */
void ChessManager::M_add_base_game_options(arg_parser_t& parser)
{
    // Add the limits argument
    parser.add_argument({"--limits", "-l"}, {
        .required = false,
        .help_message = "Set the engine search limits (e.g. \"movetime 1000 depth 5\")",
        .default_value = "",
        .type = arg_parser_t::STRING,
        .validator = [](std::string value) {
            // Check if the value is valid
            std::istringstream iss(value);
            try {
                uci::UCI::parseGoOptions(iss, false, false);
                return true;
            } catch (...) {}
            return false;
        }
    });

    // Add the FEN argument
    parser.add_argument({"--fen", "-f"}, {
        .required = false,
        .help_message = "Set the FEN string (or 'startpos' for the initial position)",
        .default_value = "",
        .type = arg_parser_t::STRING,
        .validator = [](std::string value) {
            // Check if the value is valid
            return chess::Board().loadFen(value);
        }
    });
}

/**
 * @brief Process the input parameters (FEN, engine constraints), if one of them is not set,
 * ask the user for input
 */
void ChessManager::M_process_game_options(arg_map_t& args)
{
    // Check if the user specified given parameters
    if (!args.exists("--fen"))
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
        m_engine.setPosition(args.get<std::string>("--fen"));
    }

    // Check if limits were specified
    if (!args.exists("--limits"))
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
        std::istringstream iss(args.get<std::string>("--limits"));
        m_options = uci::UCI::parseGoOptions(iss, false, false);
    }
}

/**
 * @brief Ask the user for a move and makes it, if invalid displays an error
 */
void ChessManager::M_player_move()
{
    using namespace chess;
    M_process_input(
        ">>> ",
        "Invalid move:",
        [this](std::string move_input) {
            // Make the move if it's legal
            Move m = m_engine.m_board.match(move_input);
            if (m != Move::nullMove && m_engine.m_board.isLegal(m))
                m_engine.m_board.makeMove(m);

            // Else, throw an exception
            else
                throw std::invalid_argument(move_input);
        }
    );
}

// BASE MANAGER

/**
 * @brief Setup the game loop with the given parameters
 */
void BaseManager::M_loop_setup(arg_map_t& args)
{
    M_process_param_input(args);
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