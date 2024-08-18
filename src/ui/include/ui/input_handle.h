#pragma once

#include <SFML/Graphics.hpp>
#include <iostream>

#include <cengine/cengine.h>

enum class BoardScreenState
{
    MAIN_MENU,
    PLAYER_VS_PLAYER,
    PLAYER_VS_ENGINE,
    ANALYSIS,
    TEST_ENGINE,
};

enum class InputState
{
    None,
    Select,
    Move,
    Promote
};

struct BoardWindowState{
    BoardScreenState screen_state = BoardScreenState::MAIN_MENU;
    InputState state = InputState::None;
    int from = -1;
    int to = -1;
    int move_flags = -1;
    bool move_piece = false;
    int player_color = chess::Piece::Black;
    int current_color = chess::Piece::White;
    chess::Board* board = nullptr;
};

class InputHandler
{
public:
    typedef std::function<void(int, BoardWindowState*, sf::RenderWindow*, sf::Event&)> callback_type;
    typedef std::function<void(BoardWindowState*, sf::RenderWindow*, sf::Event&)> custom_callback_type;

    InputHandler(chess::Manager* manager, BoardWindowState* state);
    InputHandler(InputHandler&& other) = default;

    void handleInput(sf::Event& event, sf::RenderWindow* window, int index, bool handle_board);
    void customEvent(custom_callback_type callback);
    void onSelected(callback_type callback);
    void onPieceMove(callback_type callback);
    void onPromotion(callback_type callback);
    void setManager(chess::Manager* manager) { m_manager = manager; }
private:
    chess::Manager* m_manager;
    BoardWindowState* m_state;  
    callback_type m_on_selected;
    callback_type m_on_piece_move;
    callback_type m_on_promotion;
    custom_callback_type m_custom_callback;
};