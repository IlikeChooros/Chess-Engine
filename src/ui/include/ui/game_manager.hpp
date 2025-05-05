#pragma once

#include "renderer.hpp"
#include <cengine/cengine.h>

UI_NAMESPACE_BEGIN

class GameManager : public Renderer
{
public:
    GameManager() = default;
    void loop(int argc = 0, char** argv = nullptr);

private:
    typedef std::function<void(std::string)> callback_t;

    std::string M_read_input();
    void M_process_input(
        const char* prompt, 
        const char* error_msg,
        callback_t validator 
    );
    void M_init();
    void M_render();
    void M_process_param_input(int argc = 0, char** argv = nullptr);
    chess::ArgParser::arg_map_t M_process_arguments(int argc, char** argv);
    void M_engine_move();
    void M_player_move();
    void M_render_gamesummary();

    chess::Engine m_engine;
    chess::SearchOptions m_options;
    chess::Result m_result;
    bool m_player_side{false}; // true for white, false for black
};


UI_NAMESPACE_END