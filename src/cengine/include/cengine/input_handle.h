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
    InputState state = InputState::None;
    int from = -1;
    int to = -1;
    int move_flags = -1;
    int player_color = chess::Piece::Black;
    int current_color = chess::Piece::White;
    chess::Board* board = nullptr;
};

void handleInput(chess::Manager* manager, sf::Event& event, sf::RenderWindow* window, BoardWindowState* state, bool handle_board);
void getBoardSize(int& size, int& offset_x, sf::RenderWindow* window);