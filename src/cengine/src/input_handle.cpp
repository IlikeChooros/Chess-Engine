#include <cengine/input_handle.h>

using namespace chess;
using namespace sf;

void doNothing(int, BoardWindowState*, sf::RenderWindow*, sf::Event&){}

InputHandler::InputHandler(Manager* manager, BoardWindowState* state)
    : m_manager(manager), m_state(state) 
{
    m_on_promotion = doNothing;
    m_on_selected = [](int index, BoardWindowState* state, sf::RenderWindow* window, sf::Event& event){
        if (Piece::getColor((*state->board)[index]) == state->current_color){
            state->state = InputState::Select;
            state->from = index;
            state->to = -1;
            state->move_flags = -1;
        }
    };
    m_on_piece_move = [this](int index, BoardWindowState* state, sf::RenderWindow* window, sf::Event& event){
        if (this->m_manager->movePiece(state->from, index, state->move_flags)){
            state->current_color ^= Piece::colorMask;
            *state->board = *this->m_manager->board();
        }
    };
}

void InputHandler::handleInput(Event& event, sf::RenderWindow* window, int index, bool handle_board)
{

    if(event.type == sf::Event::KeyPressed){
        // Print fen
        if(event.key.code == sf::Keyboard::P){
            std::cout << "fen: " << m_state->board->getFen() << '\n';
        }
    }

    if (event.type != sf::Event::MouseButtonPressed || !handle_board || index == -1){
        return;
    }

    if (m_state->state == InputState::None){
        m_on_selected(index, m_state, window, event);
    } else if (m_state->state == InputState::Select){
        m_state->to = index;
        m_state->move_flags = -1;
        // Check if that's a promotion move and the state is not already set
        if(m_manager->isPromotion(m_state->from, m_state->to) && m_state->state != InputState::Promote){
            m_state->state = InputState::Promote;
            return;
        }
        
        m_on_piece_move(index, m_state, window, event);
        m_state->state = InputState::None;
        m_state->from = -1;
        m_state->to = -1;
    } else if (m_state->state == InputState::Promote){
        m_on_promotion(index, m_state, window, event);
    }
}

void InputHandler::onSelected(callback_type callback)
{
    m_on_selected = callback;
}

void InputHandler::onPieceMove(callback_type callback)
{
    m_on_piece_move = callback;
}

void InputHandler::onPromotion(callback_type callback)
{
    m_on_promotion = callback;
}

/**
 * @brief Get the board size and offset
 * @param size The size of the board square (in pixels)
 * @param offset The x offset of the board (in pixels)
 * @param window The window to get the size from
 */
void getBoardSize(int& size, int& offset, RenderWindow* window){
    const auto window_size = window->getSize();
    size = std::min(window_size.x, window_size.y);
    offset = (window_size.x - size)/2;
    size /= 8;
}

/**
 * @brief Handle input from the ui window
 * @param manager The manager to handle the input
 * @param event The event to handle
 * @param window The window to get the input from
 */
void handleInput(Manager* manager, Event& event, RenderWindow* window, BoardWindowState* state, bool handle_board)
{
    auto prev_state = state->state;
    state->state = InputState::None;
    static int from = -1;

    if(event.type == sf::Event::KeyPressed){
        if(event.key.code == sf::Keyboard::Escape){
            window->close();
            return;
        }

        // Print fen
        if(event.key.code == sf::Keyboard::P){
            std::cout << "fen: " << manager->board()->getFen() << '\n';
        }
    }

    if (!handle_board || state->current_color != state->player_color)
        return;

    if(event.type == sf::Event::MouseButtonPressed){
        auto mouse = sf::Mouse::getPosition(*window);
        int size, offset_x;
        getBoardSize(size, offset_x, window);

        int x = (mouse.x - offset_x) / size;
        int y = mouse.y / size;
        int to = x + y*8;

        if (x < 0 || x > 7 || y < 0 || y > 7)
            return;

        if (from == -1){
            if (Piece::getColor(manager->board()->board[y*8 + x]) != state->player_color){
                return;
            }
            from = to;
            state->state = InputState::Select;
            state->from = from;
        } else {
            // Check if that's a promotion move and the state is not already set
            if(manager->isPromotion(from, to) && state->state != InputState::Promote){
                state->state = InputState::Promote;
                state->from = from;
                state->to = to;
                return;
            }
            
            if(manager->movePiece(from, to, state->move_flags)){
                state->current_color ^= Piece::colorMask;
                *state->board = *manager->board();
            }
            
            from = -1;
            state->state = InputState::Move;
            state->move_flags = -1;
        }
    } else {
        state->state = prev_state; // Do nothing
    }
}