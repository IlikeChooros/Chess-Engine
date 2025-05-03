#include <ui/game_manager.hpp>


UI_NAMESPACE_BEGIN

void GameManager::loop()
{
    M_init();
    M_process_param_input();
    M_render();

    if (m_player_side != m_engine.m_board.turn())
    {
        // Make the first engine move
        M_engine_move();
        M_render();

        if (m_engine.m_board.isTerminated())
            return;
    }
    
    // Main game loop
    do{
        // Ask the player for a move
        M_player_move();
        M_render();
        
        if (m_engine.m_board.isTerminated())
            break;
        
        // Make the engine move
        M_engine_move();
        M_render();
    } while(!m_engine.m_board.isTerminated());
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
    constexpr int max_count = 6, wait_time = 150;

    auto start_time = std::chrono::high_resolution_clock::now();
    while(m_engine.m_main_thread.is_thinking()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // Get the result
        auto result = m_engine.m_main_thread.get_result().get();

        // Print the depth and evaluation
        std::cout << "\r" << std::string(max_count + 3, ' ') << ": " << "depth " << result.depth << " eval ";
        if (result.score.type == chess::Score::mate)
        {
            std::cout << (result.score.value > 0 ? "M" : "-M") 
                << std::abs(result.score.value);
        }
        else
        {
            std::cout << std::setprecision(2) << result.score.value / 100.0;
        }

        // Print the current best move
        std::cout << " bestmove " << result.bestmove.uci() << " ";

        // Print dots as a loading bar
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now() - start_time
        ).count();

        if (elapsed > wait_time)
        {
            count += inc;
            start_time = std::chrono::high_resolution_clock::now();
        }
        
        // Change the direction of the dots
        if (count == max_count - 1)
            inc = -1;
        else if (count == 0)
            inc = 1;

        // Print the moving dot
        std::cout << "\r[" << std::string(count, ' ') << "." << std::string(max_count - count - 1, ' ') << "]";
        std::cout.flush(); // flush the output
    }

    // Print the final result with centered OK
    std::cout << "\r[" << 
        std::string(max_count / 2 - 1, ' ') 
        << "OK" 
        << std::string(max_count / 2 - 1, ' ') << "]";
    std::cout.flush();

    m_engine.m_board.makeMove(res.get().bestmove);
    std::cout << "\nEngine move: " << res.get().bestmove.uci() << "\n\n";
}

/**
 * @brief Read the input and make the valid move
 */
void GameManager::M_player_move()
{
    using namespace chess;
    std::string move;
    
    do {
        std::cout << ">>> ";
        std::getline(std::cin, move);
        
        // Check if the move is valid
        Move m = m_engine.board().match(move);
        if (m_engine.board().isLegal(m))
        {
            m_engine.board().makeMove(m);
            break;
        }
        else
        {
            if (move == "exit" || move == "quit" || move == "q")
            {
                std::cout << "Exiting...\n";
                exit(0);
            }
            else
                std::cout << "Invalid move: " << move << "\n";
        }
    } while(true);
}

/**
 * @brief Process the input parameters (FEN, side, engine constraints)
 */
void GameManager::M_process_param_input()
{
    std::cout << "CEngine UCI ver " << global_settings.version << " with UI\n";
    bool valid = false;

    do 
    {
        std::cout << "Enter the FEN string (or 'startpos' for the initial position): ";
        std::string fen;
        std::getline(std::cin, fen);
        valid = m_engine.setPosition(fen);
    } while(!valid);
    

    do
    {
        valid = true;
        std::cout << "Enter your side (w/b): ";
        std::string side;
        std::getline(std::cin, side);

        // Should be either 'w', 'b', 'white' or 'black'
        if (side == "w" || side == "white")
            m_player_side = 1;
        else if (side == "b" || side == "black")
            m_player_side = 0;
        else
            valid = false;
    } while (!valid);

    do
    {
        valid = true;
        std::cout << "Enter the constraints: ";
        std::string constraints;
        std::getline(std::cin, constraints);
        std::istringstream iss(constraints);
        
        try {
            m_options = uci::UCI::parseGoOptions(iss, false, false);
        }
        catch(const std::runtime_error& e) {
            std::cerr << "Invalid constraints: " << e.what() << "\n";
            valid = false;
        }
        
    } while (!valid);    
}

/**
 * @brief Render the board
 */
void GameManager::M_render()
{
    render(m_engine.board());
}


UI_NAMESPACE_END