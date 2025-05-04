#pragma once

#include "board_renderer.hpp"
#include <cengine/cengine.h>

UI_NAMESPACE_BEGIN

class GameManager : public BoardRenderer
{
public:
    GameManager() = default;
    void loop(int argc = 0, char** argv = nullptr);
private:
    static constexpr int max_dots_count = 6, wait_dot_time = 150; // in ms

    void M_init();
    void M_render();
    void M_process_param_input(int argc = 0, char** argv = nullptr);
    chess::ArgParser::arg_map_t M_process_arguments(int argc, char** argv);
    void M_engine_move();
    void M_render_engine_line();
    void M_render_engine_outputs();
    void M_player_move();
    void M_render_gamesummary();

    chess::Engine m_engine;
    chess::SearchOptions m_options;
    chess::Result m_result;
    bool m_player_side{false}; // true for white, false for black
};


UI_NAMESPACE_END