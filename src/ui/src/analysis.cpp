#include <ui/analysis.hpp>


UI_NAMESPACE_BEGIN

void Analysis::loop(arg_map_t& args)
{
    M_loop_setup(args);

    // Main game loop
    while(!m_engine.m_board.isTerminated()) 
    {
        // Ask the player for a move
        std::cout << Ansi::CURSOR_SHOW;
        M_player_move();
        std::cout << Ansi::CURSOR_HIDE;
        M_render();

        m_engine.go(m_options);
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

        m_result = m_engine.m_main_thread.get_result().get();
        render_engine_outputs(m_result, true);
    }
}

/**
 * @brief Add --limits and --fen options to the parser
 */
void Analysis::add_args(chess::ArgParser& parser)
{
    M_add_base_game_options(parser);
}

/**
 * @brief Render the board
 */
void Analysis::M_render()
{
    render_board(m_engine.board());
}

/**
 * @brief Process the command line arguments with ArgParser, or standard input
 */
void Analysis::M_process_param_input(arg_map_t& map)
{
    // Read the command line arguments & ask for input if needed
    M_process_game_options(map);

    // Clear the screen
    print<false, false>(CURSOR_HOME, '\r', 
        CLEAR_LINE_FROM_CURSOR, '\n', 
        CLEAR_LINE_FROM_CURSOR, '\n', 
        CLEAR_LINE_FROM_CURSOR, '\n', 
        cursor_up(3));
}



UI_NAMESPACE_END