#include <ui/game_manager.hpp>


UI_NAMESPACE_BEGIN

void GameManager::loop(int argc, char** argv)
{
    M_init();
    M_process_param_input(argc, argv);
    std::cout << CLEAR_SCREEN; // Clear the screen
    M_render();

    if (m_player_side != m_engine.m_board.turn())
    {
        // Make the first engine move
        M_engine_move();
        std::cout << Ansi::CURSOR_HIDE;
        M_render();
        render_engine_outputs(m_result);
    }
    
    // Main game loop
    while(!m_engine.m_board.isTerminated()) 
    {
        // Ask the player for a move
        std::cout << Ansi::CURSOR_SHOW;
        M_player_move();
        std::cout << Ansi::CURSOR_HIDE;
        M_render();
        flush(); // flush the output
        
        if (m_engine.m_board.isTerminated())
            break;
        
        // Make the engine move
        M_engine_move();
        M_render();
        render_engine_outputs(m_result);
    }

    // Print the final result
    std::cout << Ansi::CURSOR_SHOW;
    M_render_gamesummary();
}

/**
 * @brief Initialize the game with start position
 */
void GameManager::M_init()
{
    chess::init();
    glogger.setPrint(false);
    glogger.setLog(false);
    m_engine.setPosition();
}

/**
 * @brief Read from standard input
 */
std::string GameManager::M_read_input()
{
    std::string input;
    std::getline(std::cin, input);
    return input;
}

/**
 * @brief Make the engine move
 */
void GameManager::M_engine_move()
{
    auto& res = m_engine.go(m_options);
    int count = 0, prevcount = 0, direction = 1;

    // Start the timer for the loading bar
    auto start_time = std::chrono::high_resolution_clock::now();
    while(m_engine.m_main_thread.is_thinking()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Get the result
        m_result = m_engine.m_main_thread.get_result().get();
        render_engine_line(m_result);

        // Print dots as a loading bar
        // clear the previous dot
        if (count != prevcount)
            print<false, false>(
                "\r[", cursor_forward(prevcount), ' ', '\r',
                cursor_forward(max_dots_count + 1), ']');
        
        print<false, false>(
            "\r[", cursor_forward(count), '.', '\r',
            cursor_forward(max_dots_count + 1), ']');


        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - start_time
        ).count();
        
        // Check if the elapsed time is greater than the wait time
        if (elapsed > wait_dot_time)
        {
            prevcount = count;
            count += direction;
            start_time = std::chrono::high_resolution_clock::now();
        }
        
        // Change the direction of the dots
        if (count == max_dots_count - 1)
            direction = -1;
        else if (count == 0)
            direction = 1;
    }

    // Make the move
    m_engine.m_board.makeMove(m_result.bestmove);
}

void GameManager::M_render_gamesummary()
{
    using namespace chess;

    print("\n", Ansi::BOLD, Ansi::UNDERLINE, "Game Summary\n\n");
    print(Ansi::BOLD, 
        ((!m_engine.m_board.turn() == m_player_side) ?
        std::string(OK_FG) + "You" : 
        std::string(ERROR_FG) + "Engine"),
        " won the game!\n"
    );
    
    // Ask if the user wants to see PGN 
    std::cout << "\nDo you want to see the PGN? (y/n) ";
    auto answer = M_read_input();

    if (answer == "y" || answer == "Y")
    {
        print(Ansi::BOLD, "\nPGN:\n\n");

        PGN pgn;
        pgn.generate_fields(m_engine.m_board);
        pgn[(m_player_side ? "White" : "Black")] = "You";
        pgn[(m_player_side ? "Black" : "White")] = "Engine";
        std::string pgn_string = pgn.pgn(m_engine.m_board);

        // Print the PGN
        std::cout << pgn_string << std::endl;
    }
}

/**
 * @brief Read the input and make the valid move
 */
void GameManager::M_player_move()
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

/**
 * @brief Process the input parameters (FEN, side, engine constraints)
 * @param argc Number of arguments
 * @param argv Array of arguments
 */
chess::ArgParser::arg_map_t GameManager::M_process_arguments(int argc, char** argv)
{
    chess::ArgParser parser(argc, argv);

    parser.addArg("--ui", "true", 
        chess::ArgParser::defaultValidator, 
            "Enable the UI (default: true)");

    parser.addArg("--fen", "", 
        [](std::string fen) { return chess::Board().loadFen(fen); }, 
            "FEN string to set the position");

    parser.addArg("--side", "", 
        [](std::string side) { return side == "w" || side == "b"; }, 
            "Player side (w/b)");
    
    parser.addArg("--limits", "",
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

    return parser.parse(); 
}

/**
 * @brief Process the standard input, with messages
 * @param prompt The prompt to display, e.g. "Enter your move: "
 * @param validator The function to validate the input, should throw an exception on error
 * @param error_msg The error message to display on invalid input
 */
void GameManager::M_process_input(
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

/**
 * @brief Process the input parameters (FEN, side, engine constraints)
 */
void GameManager::M_process_param_input(int argc, char** argv)
{
    // Process the command line arguments
    auto args = M_process_arguments(argc, argv);

    // Helper lambda function to check if an argument exists
    auto exists = [](chess::ArgParser::arg_map_t& args, const std::string& name) {
        return args.find(name) != args.end();
    };

    // Print out the version
    std::cout << "CEngine UCI ver " << global_settings.version << " with UI\n";

    // Check if the user specified given parameters
    if (!exists(args, "--fen"))
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

    if (!exists(args, "--side"))
    {
        M_process_input(
            "Enter your side (w/b): ",
            "Invalid side:",
            [this](std::string side) {
                if (side == "w" || side == "white")
                    m_player_side = 1;
                else if (side == "b" || side == "black")
                    m_player_side = 0;
                else
                    throw std::runtime_error(side);
            }
        );
    } else {
        m_player_side = (args["--side"] == "w");
    }

    if (!exists(args, "--limits"))
    {
        M_process_input(
            "Enter the constraints: ",
            "Invalid constraints:",
            [this](std::string constraints) {
                std::istringstream iss(constraints);
                m_options = uci::UCI::parseGoOptions(iss, false, false);
            }
        );
    } else {
        std::istringstream iss(args["--limits"]);
        m_options = uci::UCI::parseGoOptions(iss, false, false);
    }
}

/**
 * @brief Render the board
 */
void GameManager::M_render()
{
    render_board(m_engine.board(), m_player_side);
}


UI_NAMESPACE_END