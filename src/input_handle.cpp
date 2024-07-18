#include "input_handle.h"

using namespace chess;
using namespace sf;

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
void handleInput(Manager* manager, Event& event, RenderWindow* window, BoardWindowState* state)
{
    auto prev_state = state->state;
    state->state = InputState::None;
    static int from = -1;

    if(event.type == sf::Event::MouseButtonPressed){
        auto mouse = sf::Mouse::getPosition(*window);
        int size, offset_x;
        getBoardSize(size, offset_x, window);

        int x = (mouse.x - offset_x) / size;
        int y = mouse.y / size;

        if (x < 0 || x > 7 || y < 0 || y > 7)
            return;

        if (from == -1){
            if (Piece::getColor(manager->board->board[y*8 + x]) != manager->getSide()){
                return;
            }
            from = x + y*8;
            state->state = InputState::Select;
            state->from = from;
        } else {
            dlogf("From: %s to: %s\n", 
                square_to_str(from).c_str(), 
                square_to_str(x + y*8).c_str()
            );

            manager->movePiece(from, x + y*8);
            from = -1;
            state->state = InputState::Move;
        }
    } else {
        state->state = prev_state; // Do nothing
    }
}