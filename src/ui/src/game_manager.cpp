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
        M_render_engine_outputs();
    }
    
    // Main game loop
    while(!m_engine.m_board.isTerminated()) 
    {
        // Ask the player for a move
        std::cout << Ansi::CURSOR_SHOW;
        M_player_move();
        std::cout << Ansi::CURSOR_HIDE;
        M_render();
        std::cout.flush(); // flush the output
        
        if (m_engine.m_board.isTerminated())
            break;
        
        // Make the engine move
        M_engine_move();
        M_render();
        M_render_engine_outputs();
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
 * @brief Make the engine move
 */
void GameManager::M_engine_move()
{
    auto& res = m_engine.go(m_options);
    int count = 0, inc = 1;

    // Start the timer for the loading bar
    auto start_time = std::chrono::high_resolution_clock::now();
    while(m_engine.m_main_thread.is_thinking()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Get the result
        m_result = m_engine.m_main_thread.get_result().get();
        M_render_engine_line();

        // Print dots as a loading bar
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - start_time
        ).count();

        // Check if the elapsed time is greater than the wait time
        if (elapsed > wait_dot_time)
        {
            count += inc;
            start_time = std::chrono::high_resolution_clock::now();
        }
        
        // Change the direction of the dots
        if (count == max_dots_count - 1)
            inc = -1;
        else if (count == 0)
            inc = 1;

        // Print the moving dot
        std::cout << "\r[" << std::string(count, ' ') << "." 
                  << std::string(max_dots_count - count - 1, ' ') << "]";
        std::cout.flush(); // flush the output
    }

    // Make the move
    m_engine.m_board.makeMove(m_result.bestmove);
}

void GameManager::M_render_gamesummary()
{
    using namespace chess;

    std::cout << "\n\n" << Ansi::BOLD << (
        (!m_engine.m_board.turn() == m_player_side) ?  
        Ansi::fg_rgb(157, 249, 169) + "You" : 
        Ansi::fg_rgb(247, 140, 131) + "Engine")
        << " won the game!\n" << Ansi::RESET;
    
    // Ask if the user wants to see PGN 
    std::cout << "\nDo you want to see the PGN? (y/n) ";
    std::string answer;
    std::getline(std::cin, answer);

    if (answer == "y" || answer == "Y")
    {
        std::cout << Ansi::BOLD << "\nPGN:\n\n" << Ansi::RESET;

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
 * @brief Render the engine line: depth, eval, bestmove
 */
void GameManager::M_render_engine_line()
{
    using namespace chess;
    std::cout << "\r" << Ansi::fg_rgb(192, 192, 192) << std::string(max_dots_count + 3, ' ') << ": depth "
        << m_result.depth << " eval ";

    if (m_result.score.type == Score::mate)
    {
        std::cout << (m_result.score.value > 0 ? "M" : "-M") 
            << std::abs(m_result.score.value);
    }
    else
    {
        std::cout << std::setprecision(2) << m_result.score.value / 100.0;
    }

    // Print the current best move
    std::cout << " bestmove " << m_result.bestmove.uci() 
        << Ansi::CLEAR_LINE_FROM_CURSOR << Ansi::RESET;
}

void GameManager::M_render_engine_outputs()
{
    if (m_result.bestmove == chess::Move::nullMove)
        return;

    // Print the final result with centered OK
    M_render_engine_line();
    std::cout << "\r" << Ansi::bg_rgb(4, 86, 15) << Ansi::fg_rgb(157, 249, 169) 
        << "[" << std::string(max_dots_count / 2 - 1, ' ') 
        << "OK" << std::string(max_dots_count / 2 - 1, ' ') << "]" 
        << Ansi::RESET << "\n";

    std::cout << Ansi::fg_rgb(192, 192, 192) 
        << "Engine move: " << m_result.bestmove.uci() 
        << Ansi::RESET << std::endl;
}

/**
 * @brief Read the input and make the valid move
 */
void GameManager::M_player_move()
{
    using namespace chess;
    std::string move_input; // Renamed to avoid confusion with chess::Move
    std::string error_msg = "";

    do {
        // Print prompt, move cursor to start, clear rest of line
        std::cout << "\r>>> " << CLEAR_LINE_FROM_CURSOR;
        std::cout.flush(); // Make sure prompt appears before waiting for input

        if (!std::getline(std::cin, move_input)) {
             // Handle EOF or input error if necessary
             std::cerr << "\nInput stream error or EOF detected. Exiting." << std::endl;
             exit(1);
        }

        // Handle exit commands first
        if (move_input == "exit" || move_input == "quit" || move_input == "q")
        {
            std::cout << "\nExiting...\n"; // Ensure newline before exit message
            exit(0);
        }

        // Attempt to parse and validate the move
        Move m = m_engine.board().match(move_input);
        if (m != Move::nullMove && m_engine.board().isLegal(m))
        {
            m_engine.board().makeMove(m);
            // Valid move made. Clear the input line entirely before returning.
            std::cout << "\r" << CLEAR_LINE_FROM_CURSOR;
            std::cout.flush();
            break; // Exit the loop
        }
        else
        {
            // Invalid move. Print message. The cursor stays after the message.
            // The next loop iteration will overwrite it starting with \r>>>
            std::cout << Ansi::fg_rgb(247, 140, 131) << Ansi::bg_rgb(86, 11, 4) << "Invalid move:" <<
                Ansi::BG_DEFAULT << " " << move_input << CLEAR_LINE_FROM_CURSOR << Ansi::RESET;
            
            // Go back up one line to overwrite the prompt
            std::cout << "\r" << Ansi::cursor_up();
        }
    } while(true);
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
 * @brief Process the input parameters (FEN, side, engine constraints)
 */
void GameManager::M_process_param_input(int argc, char** argv)
{
    auto args = M_process_arguments(argc, argv);
    auto exists = [](chess::ArgParser::arg_map_t& args, const std::string& name) {
        return args.find(name) != args.end();
    };

    if (exists(args, "--fen"))
        m_engine.setPosition(args["--fen"]);

    if (exists(args, "--side"))
        m_player_side = (args["--side"] == "w");

    if (exists(args, "--limits"))
    {
        std::istringstream iss(args["--limits"]);
        m_options = uci::UCI::parseGoOptions(iss, false, false);
    }

    const auto error_fg = Ansi::fg_rgb(247, 140, 131);
    const auto error_bg = Ansi::bg_rgb(86, 11, 4);

    std::cout << "CEngine UCI ver " << global_settings.version << " with UI\n";
    bool valid = true;
    if (!exists(args, "--fen"))
    {
        do 
        {
            std::cout << "\r" << Ansi::BOLD 
                << "Enter the FEN string (or 'startpos' for the initial position): "
                << CLEAR_LINE_FROM_CURSOR << Ansi::RESET;
            std::cout.flush(); // flush the output

            std::string fen;
            std::getline(std::cin, fen);
            valid = m_engine.setPosition(fen);

            if (!valid)
            {
                std::cout << error_fg << error_bg 
                    << "\rInvalid FEN string:" << Ansi::BG_DEFAULT 
                    << " " << fen << CLEAR_LINE_FROM_CURSOR << Ansi::RESET;
                
                // Go back up one line to overwrite the prompt
                std::cout << "\r" << Ansi::cursor_up();
            }

        } while(!valid);

        // Clear the invalid input line
        std::cout << "\r" << Ansi::CLEAR_LINE_FROM_CURSOR;
        std::cout << Ansi::cursor_up();
        std::cout.flush(); // flush the output
    }

    if (!exists(args, "--side"))
    {
        do
        {
            valid = true;
            std::cout << "\r" << Ansi::BOLD 
                << "Enter your side (w/b): " 
                << CLEAR_LINE_FROM_CURSOR << Ansi::RESET;
            std::cout.flush(); // flush the output

            std::string side;
            std::getline(std::cin, side);

            // Should be either 'w', 'b', 'white' or 'black'
            if (side == "w" || side == "white")
                m_player_side = 1;
            else if (side == "b" || side == "black")
                m_player_side = 0;
            else
            {
                valid = false;

                // Print error message
                std::cout << error_fg << error_bg 
                    << "\rInvalid side:" << Ansi::BG_DEFAULT 
                    << " " << side << CLEAR_LINE_FROM_CURSOR << Ansi::RESET;
                
                // Go back up one line to overwrite the prompt
                std::cout << "\r" << Ansi::cursor_up();
            }
        } while (!valid);

        std::cout << "\r" << Ansi::CLEAR_LINE_FROM_CURSOR;
        std::cout << Ansi::cursor_up();
        std::cout.flush(); // flush the output
    }

    if (!exists(args, "--limits"))
    {
        do
        {
            valid = true;
            std::cout << "\r" << BOLD << "Enter the constraints: " 
                << CLEAR_LINE_FROM_CURSOR << Ansi::RESET;
            std::cout.flush(); // flush the output
            
            std::string constraints;
            std::getline(std::cin, constraints);
            std::istringstream iss(constraints);
            
            try {
                m_options = uci::UCI::parseGoOptions(iss, false, false);
            }
            catch(std::exception& e) {

                std::cout << error_fg << error_bg 
                    << "\rInvalid constraints:" << Ansi::BG_DEFAULT 
                    << " " << e.what() << CLEAR_LINE_FROM_CURSOR << Ansi::RESET;
                
                // Go back up one line to overwrite the prompt
                std::cout << "\r" << Ansi::cursor_up();
                
                valid = false;
            }
            
        } while (!valid);

        // Clear the invalid input line
        std::cout << "\r" << Ansi::CLEAR_LINE_FROM_CURSOR;
        std::cout << Ansi::cursor_up();
        std::cout.flush(); // flush the output
    } 
}

/**
 * @brief Render the board
 */
void GameManager::M_render()
{
    render(m_engine.board(), m_player_side);
}


UI_NAMESPACE_END