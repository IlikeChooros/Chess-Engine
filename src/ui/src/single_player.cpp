#include <ui/single_player.hpp>


UI_NAMESPACE_BEGIN

void SinglePlayer::loop(arg_map_t& args)
{
    M_loop_setup(args);
    
    // Main game loop
    while(!m_engine.m_board.isTerminated()) 
    {
        // Ask the player for a move
        while ((m_engine.m_board.turn() == m_player_side) && 
            (!m_engine.m_board.isTerminated()))
        {
            std::cout << Ansi::CURSOR_SHOW;
            M_player_move();
            std::cout << Ansi::CURSOR_HIDE;
            M_render();
            flush(); // flush the output
        }
        
        if (m_engine.m_board.isTerminated())
            break;
        
        // Make the engine move
        std::cout << Ansi::CURSOR_HIDE;
        M_engine_move();
        M_render();
        render_engine_outputs(m_result);
    }

    // Print the final result
    std::cout << Ansi::CURSOR_SHOW;
    M_render_gamesummary();
}

/**
 * @brief Clear the screen
 */
void SinglePlayer::clear()
{
    BaseManager::clear();
}

/**
 * @brief Add the command line arguments (--side), --limits and --fen should 
 * be already added to the parser
 */
void SinglePlayer::add_args(chess::ArgParser& parser)
{
    parser.add_argument({"--side"}, {
        .required = false,
        .help_message = "Set the player side (w/b)",
        .default_value = "",
        .type = chess::ArgParser::STRING,
        .validator = [](std::string value) {
            return value == "w" || value == "b" || 
                   value == "white" || value == "black";
        },
        .action = [](chess::ArgParser::arg_value_t& value, std::string arg) {
            if (arg == "w" || arg == "white")
                value = true;
            else if (arg == "b" || arg == "black")
                value = false;
        }
    });
}

/**
 * @brief Render the board
 */
void SinglePlayer::M_render()
{
    render_board(m_engine.board(), m_player_side);
}


/**
 * @brief Make the engine move
 */
void SinglePlayer::M_engine_move()
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

/**
 * @brief Display the game summary, optionally show the PGN,
 *  should be called after termination
 */
void SinglePlayer::M_render_gamesummary()
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
 * @brief Process the input parameters (FEN, side, engine constraints)
 */
void SinglePlayer::M_process_param_input(chess::ArgParser::arg_map_t& args)
{
    using namespace chess;

    // Print out the version
    std::cout << "CEngine UCI ver " << global_settings.version << " with UI\n";

    M_process_game_options(args);

    if (!args.exists("--side"))
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
        m_player_side = args.get<bool>("--side");
    }

    // Remove 3 lines (version, input, error msg)
    print<false, false>(CURSOR_HOME, '\r', 
        CLEAR_LINE_FROM_CURSOR, '\n', 
        CLEAR_LINE_FROM_CURSOR, '\n', 
        CLEAR_LINE_FROM_CURSOR, '\n', 
        cursor_up(3));
}


UI_NAMESPACE_END