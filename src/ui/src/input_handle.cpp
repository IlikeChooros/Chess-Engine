#include <ui/input_handle.h>

using namespace chess;
using namespace sf;

void doNothing(int, BoardWindowState*, sf::RenderWindow*, sf::Event&){}
void doNothingCustom(BoardWindowState*, sf::RenderWindow*, sf::Event&){}

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
        if (this->m_manager->makeMove(state->from, index, state->move_flags)){
            state->current_color ^= Piece::colorMask;
            *state->board = *this->m_manager->board();
        }
    };
    m_custom_callback = doNothingCustom;
}

void InputHandler::handleInput(Event& event, sf::RenderWindow* window, int index, bool handle_board)
{

    m_custom_callback(m_state, window, event);

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

void InputHandler::customEvent(custom_callback_type callback)
{
    m_custom_callback = callback;
}