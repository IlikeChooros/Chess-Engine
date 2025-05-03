#pragma once

#include "board_renderer.hpp"
#include <cengine/cengine.h>

UI_NAMESPACE_BEGIN

class GameManager : public BoardRenderer
{
public:
    GameManager() = default;
    void loop();
private:
    void M_init();
    void M_render();
    void M_process_param_input();
    void M_engine_move();
    void M_player_move();

    chess::Engine m_engine;
    chess::SearchOptions m_options;
    bool m_player_side{false}; // true for white, false for black
};


UI_NAMESPACE_END