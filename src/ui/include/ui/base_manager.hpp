#pragma once

#include "renderer.hpp"
#include <cengine/cengine.h>

UI_NAMESPACE_BEGIN

// Base class for all UI managers
// Requires implementation of:
// - loop
// - process_param_input
// - process_arguments
// - render
class BaseManager : public Renderer
{
public:
    typedef chess::ArgParser::arg_map_t arg_map_t;
    typedef chess::ArgParser arg_parser_t;

    BaseManager() = default;
    BaseManager(const BaseManager&) = delete;
    BaseManager& operator=(const BaseManager&) = delete;
    BaseManager(BaseManager&&) = default;
    BaseManager& operator=(BaseManager&&) = default;
    ~BaseManager() { clear(); }

    /**
     * @brief Main loop of the UI, should first call the `M_loop_setup` method
     */
    virtual void loop(arg_map_t& map) = 0;

    /**
     * @brief Clear the screen and reset the cursor
     */
    virtual void clear()
    {
        std::cout << Ansi::RESET << CLEAR_SCREEN << Ansi::CURSOR_HOME;
        std::cout.flush(); // flush the output
    }

protected:
    typedef std::function<void(std::string)> callback_t;

    static void M_process_input(
        const char* prompt, 
        const char* error_msg,
        callback_t validator 
    );
    static std::string M_read_input();
    void M_loop_setup(arg_map_t& args);

    /**
     * @brief Main rendering function (for example displaying the board)
     */
    virtual void M_render() = 0;

    /**
     * @brief Process the command line arguments with ArgParser, or standard input
     */
    virtual void M_process_param_input(arg_map_t& map) = 0;
};

// Adds SearachOptions and Result to the BaseManager
class ChessManager : public BaseManager
{
public:
    ChessManager() = default;

    static void add_args(arg_parser_t& parser)
    {
        M_add_base_engine_options(parser);
        M_add_base_game_options(parser);
    }
protected:
    // Adds --hash, --logfile, --threads options
    static void M_add_base_engine_options(arg_parser_t& parser);

    // Checks if the --hash, --logfile, --threads options are set
    void M_process_engine_options(arg_map_t& args);

    // Adds --limits & --fen options
    static void M_add_base_game_options(arg_parser_t& parser);

    // Checks if the --limits and --fen options are set
    void M_process_game_options(arg_map_t& args);

    // Ask the user for a move
    void M_player_move();

    chess::Engine m_engine;
    chess::SearchOptions m_options;
    chess::Result m_result;
};

UI_NAMESPACE_END