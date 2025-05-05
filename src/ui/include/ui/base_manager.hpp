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

    BaseManager() = default;
    BaseManager(const BaseManager&) = delete;
    BaseManager& operator=(const BaseManager&) = delete;
    BaseManager(BaseManager&&) = default;
    BaseManager& operator=(BaseManager&&) = default;
    ~BaseManager() { clear(); }

    /**
     * @brief Main loop of the UI, should first call the `M_loop_setup` method
     */
    virtual void loop(int argc = 0, char** argv = nullptr) = 0;

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

    void M_process_input(
        const char* prompt, 
        const char* error_msg,
        callback_t validator 
    );
    virtual void M_init();
    std::string M_read_input();
    void M_loop_setup(int argc, char** argv);

    /**
     * @brief Main rendering function (for example displaying the board)
     */
    virtual void M_render() = 0;

    /**
     * @brief Process the command line arguments with ArgParser, or standard input
     */
    virtual void M_process_param_input(int argc = 0, char** argv = nullptr) = 0;

    /**
     * @brief Process the command line arguments
     */
    virtual chess::ArgParser::arg_map_t M_process_arguments(int argc, char** argv) = 0;

    chess::Engine m_engine;
    chess::ArgParser m_parser;
};

// Adds SearachOptions and Result to the BaseManager
class ChessManager : public BaseManager
{
public:
    ChessManager() = default;
protected:
    // Adds --hash, --logfile, --threads options
    void M_add_base_engine_options();

    // Checks if the --hash, --logfile, --threads options are set
    void M_process_engine_options(chess::ArgParser::arg_map_t& args);

    // Adds --limits & --fen options
    void M_add_base_game_options();

    // Checks if the --limits and --fen options are set
    void M_process_game_options(chess::ArgParser::arg_map_t& args);

    chess::SearchOptions m_options;
    chess::Result m_result;
};

UI_NAMESPACE_END