#include <ui/game_manager.hpp>


UI_NAMESPACE_BEGIN

void GameManager::loop()
{
    M_init();
    M_process_param_input();

    if (m_player_side != m_engine.m_board.turn())
    {
        // Make the first engine move
        M_render();
        auto& res = m_engine.go(m_options);
        m_engine.join();
        m_engine.m_board.makeMove(res.get().bestmove);
    }
    
    // Main game loop
    do{
        M_render();
    }
}


void GameManager::M_init()
{
    chess::init();
    m_engine.setPosition();
}

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

void GameManager::M_render()
{
    render(m_engine.board());
}


UI_NAMESPACE_END