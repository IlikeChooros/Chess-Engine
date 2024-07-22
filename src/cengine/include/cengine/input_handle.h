#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>

#include "manager.h"
#include "utils.h"

enum class InputState
{
    None,
    Select,
    Move
};

struct BoardWindowState{
    InputState state;
    int from;
};

void handleInput(chess::Manager* manager, sf::Event& event, sf::RenderWindow* window, BoardWindowState* state);
void getBoardSize(int& size, int& offset_x, sf::RenderWindow* window);