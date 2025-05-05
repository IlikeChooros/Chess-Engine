#pragma once

#include "namespaces.hpp"
#include "ansi.hpp"
#include <cengine/engine.h>

UI_NAMESPACE_BEGIN 


// Class to render the chess board in command line
class Renderer : public Ansi
{
public:
    typedef Ansi::string_t string_t;
    typedef std::initializer_list<string_t> args_t;

    static constexpr const char* ERROR_FG = ANSI_FG_COLOR_RGB(247, 140, 131);
    static constexpr const char* ERROR_BG = ANSI_BG_COLOR_RGB(86, 11, 4);
    static constexpr const char* OK_FG = ANSI_FG_COLOR_RGB(157, 249, 169);
    static constexpr const char* OK_BG = ANSI_BG_COLOR_RGB(4, 86, 15);
    static constexpr int max_dots_count = 6, wait_dot_time = 150; // in ms

    Renderer() = default;
    ~Renderer() = default;

    static void render_board(const chess::Board& board, bool side = 1);
    static void render_engine_line(const chess::Result& result);
    static void render_engine_outputs(const chess::Result& result);

    // Flush the output
    static void flush()
    {
        std::cout.flush(); // flush the output
    }

    // Sets the cursor to begining of the line and clears the line
    static void clear_line()
    {
        std::cout << "\r" << Ansi::CLEAR_LINE_FROM_CURSOR;
        std::cout.flush(); // flush the output
    }

    // Moves the cursor to the beginning of the line and moves it up
    static void move_cursor_up(int line = 1, bool clear = false)
    {
        std::cout << "\r";
        if (clear)
            std::cout << Ansi::CLEAR_LINE_FROM_CURSOR;

        std::cout << Ansi::cursor_up(line);
        std::cout.flush(); // flush the output
    }

    /**
     * @brief Print a message with optional colors and styles
     * @tparam Clear Whether to clear the line after printing (default=true)
     * @tparam Reset Whether to reset the colors after printing (default=true)
     * @param args The message to print, can be a mix of strings and ANSI codes
     */
    template <bool Clear = true, bool Reset = true, typename... Args>
    static void print(Args... args)
    {
        ((std::cout << args), ...);

        if constexpr (Reset)
            std::cout << Ansi::RESET;

        if constexpr (Clear)
            std::cout << Ansi::CLEAR_LINE_FROM_CURSOR;
    }

    // Print an error message
    static void print_error(const std::string& main_reason, const std::string& desc = "")
    {
        print("\r", ERROR_FG, ERROR_BG, main_reason, Ansi::BG_DEFAULT, " " + desc);
    }

    // Print an info message
    static void print_ok(const std::string& main_reason, const std::string& desc = "")
    {
        print("\r", OK_FG, OK_BG, main_reason, Ansi::BG_DEFAULT, desc);
    }

private:

    // Helper function to create a string of spaces
    static std::string M_make_spaces(int count)
    {
        return std::string(count, ' ');
    }

    // Render a single line of the chess board
    static void renderLine(const chess::Board& board, int row, bool side, std::ostream& os);
};

UI_NAMESPACE_END