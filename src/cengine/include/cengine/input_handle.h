#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>

#include "manager.h"
#include "utils.h"

enum class InputState
{
    None,
    Select,
    Move,
    Promote
};

struct BoardWindowState{
    InputState state;
    int from;
    int to;
    int move_flags = -1;
};

void handleInput(chess::Manager* manager, sf::Event& event, sf::RenderWindow* window, BoardWindowState* state);
void getBoardSize(int& size, int& offset_x, sf::RenderWindow* window);