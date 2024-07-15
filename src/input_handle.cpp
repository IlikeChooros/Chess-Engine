#include "input_handle.h"

using namespace chess;
using namespace sf;

void getBoardSize(int& size, int& offset, RenderWindow* window){
    const auto window_size = window->getSize();
    size = std::min(window_size.x, window_size.y);
    offset = (window_size.x - size)/2;
    size /= 8;
}

void handleInput(Manager* manager, Event& event, RenderWindow* window)
{
    static int from = -1;

    if(event.type == sf::Event::MouseButtonPressed){
        auto mouse = sf::Mouse::getPosition(*window);
        int size, offset_x;
        getBoardSize(size, offset_x, window);

        int x = (mouse.x - offset_x) / size;
        int y = 7 - mouse.y / size;

        if (x < 0 || x > 7 || y < 0 || y > 7)
            return;

        if (from == -1){
            from = x + y*8;
        } else {
            manager->movePiece(from, x + y*8);
            from = -1;
        }
    }

    if(event.type == sf::Event::MouseButtonReleased){
        
    }
}