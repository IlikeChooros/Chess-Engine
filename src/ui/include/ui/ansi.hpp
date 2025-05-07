#pragma once

#include <iostream>
#include <string>
#include <stdarg.h>
#include <vector>
#include <algorithm> // for std::clamp

#include "namespaces.hpp"

UI_NAMESPACE_BEGIN


// RGB color macros for ANSI escape codes for constexpr usage
#define ANSI_FG_COLOR_RGB(r, g, b) "\033[38;2;" #r ";" #g ";" #b "m"
#define ANSI_BG_COLOR_RGB(r, g, b) "\033[48;2;" #r ";" #g ";" #b "m"

// Ansi escape codes for terminal text formatting
class Ansi
{
    // Helper function to format strings with variable arguments
    static std::string FORMAT(const char* format, ...) {
        va_list args;
        va_start(args, format);
        va_list args_copy;
        va_copy(args_copy, args); // Need a copy for the second vsnprintf call

        // Determine required buffer size
        int size = std::vsnprintf(nullptr, 0, format, args);
        va_end(args);

        if (size < 0) {
             va_end(args_copy);
             return ""; // Formatting error
        }

        // Create buffer and format
        std::vector<char> buffer(size + 1);
        std::vsnprintf(buffer.data(), buffer.size(), format, args_copy);
        va_end(args_copy);

        return std::string(buffer.data());
    }

public:

    typedef std::string string_t;

    // ----- ANSI escape codes for text formatting -----

    // ANSI: Reset all attributes, useful to reset after applying styles
    static constexpr const char* RESET          = "\033[0m";

    // ANSI: Bold text
    static constexpr const char* BOLD           = "\033[1m";

    // ANSI: Underline text
    static constexpr const char* UNDERLINE      = "\033[4m";

    // Reverse video (white text on black background)
    static constexpr const char* REVERSE        = "\033[7m";
    
    // ----- Foreground colors -----

    static constexpr const char* FG_BLACK       = "\033[30m";
    static constexpr const char* FG_RED         = "\033[31m";
    static constexpr const char* FG_GREEN       = "\033[32m";
    static constexpr const char* FG_YELLOW      = "\033[33m";
    static constexpr const char* FG_BLUE        = "\033[34m";
    static constexpr const char* FG_MAGENTA     = "\033[35m";
    static constexpr const char* FG_CYAN        = "\033[36m";
    static constexpr const char* FG_WHITE       = "\033[37m";
    static constexpr const char* FG_LIGHT_GRAY  = ANSI_FG_COLOR_RGB(192, 192, 192);
    static constexpr const char* FG_DEFAULT     = "\033[39m"; // Default foreground color

    /**
     * @brief Get the ANSI escape code for a specific foreground color.
     * @param color The color code (0-255)
     */
    static string_t fg_color(int color) {
        return FORMAT("\033[38;5;%dm", color);
    }

    /**
     * @brief Get the ANSI escape code for a specific RGB color.
     * @param r The red component (0-255)
     * @param g The green component (0-255)
     * @param b The blue component (0-255)
     */
    static string_t fg_rgb(int r, int g, int b) {
        r = std::clamp(r, 0, 255);
        g = std::clamp(g, 0, 255);
        b = std::clamp(b, 0, 255);
        return FORMAT("\033[38;2;%d;%d;%dm", r, g, b);
    }

    // ----- Background colors -----
    static constexpr const char* BG_BLACK       = "\033[40m";
    static constexpr const char* BG_RED         = "\033[41m";
    static constexpr const char* BG_GREEN       = "\033[42m";
    static constexpr const char* BG_YELLOW      = "\033[43m";
    static constexpr const char* BG_BLUE        = "\033[44m";
    static constexpr const char* BG_MAGENTA     = "\033[45m";
    static constexpr const char* BG_CYAN        = "\033[46m";
    static constexpr const char* BG_WHITE       = "\033[47m";
    static constexpr const char* BG_DEFAULT     = "\033[49m"; // Default background color

    /**
     * @brief Get the ANSI escape code for a specific background color.
     * @param color The color code (0-255)
     */
    static string_t bg_color(int color) {
        return FORMAT("\033[48;5;%dm", color);
    }

    /**
     * @brief Get the ANSI escape code for a specific RGB background color.
     * @param r The red component (0-255)
     * @param g The green component (0-255)
     * @param b The blue component (0-255)
     */
    static string_t bg_rgb(int r, int g, int b) {
        r = std::clamp(r, 0, 255);
        g = std::clamp(g, 0, 255);
        b = std::clamp(b, 0, 255);
        return FORMAT("\033[48;2;%d;%d;%dm", r, g, b);
    }

    // ----- Escape codes for screen clearing -----

    // ANSI: Clear entire screen and move cursor to home position
    static constexpr const char* CLEAR_SCREEN           = "\033[2J";

    // ANSI: Clear screen from cursor to end
    static constexpr const char* CLEAR_SCREEN_FROM_CURSOR = "\033[J";

    // ANSI: Clear screen from cursor to end
    static constexpr const char* CLEAR_SCREEN_TO_CURSOR = "\033[1J";

    // ANSI: Clear line from cursor to beginning and move cursor to beginning
    static constexpr const char* CLEAR_LINE             = "\033[2K";

    // ANSI: Clear line from cursor to end
    static constexpr const char* CLEAR_LINE_FROM_CURSOR = "\033[K";

    // ANSI: Clear line from cursor to beginning
    static constexpr const char* CLEAR_LINE_TO_CURSOR   = "\033[1K";


    // ----- ANSI escape codes for cursor control -----

    // ANSI: Move cursor to home position (top-left corner)
    static constexpr const char* CURSOR_HOME    = "\033[H";

    // ANSI: Save cursor position, may be used to restore it later
    static constexpr const char* CURSOR_SAVE    = "\033[s";

    // ANSI: Restore cursor position
    static constexpr const char* CURSOR_RESTORE = "\033[u";

    // ANSI: Hide cursor
    static constexpr const char* CURSOR_HIDE    = "\033[?25l";

    // ANSI: Show cursor
    static constexpr const char* CURSOR_SHOW    = "\033[?25h";

    // ANSI: Move cursor up N lines
    static string_t cursor_up(size_t N = 1) {
        if (N == 0) return "";
        return FORMAT("\033[%zuA", N);
    }

    // ANSI: Move cursor down N lines
    static string_t cursor_down(size_t N = 1) {
        if (N == 0) return "";
        return FORMAT("\033[%zuB", N);
    }

    // ANSI: Move cursor forward N columns
    static string_t cursor_forward(size_t N = 1) {
        if (N == 0) return "";
        return FORMAT("\033[%zuC", N);
    }

    // ANSI: Move cursor backward N columns
    static string_t cursor_back(size_t N = 1) {
        if (N == 0) return "";
        return FORMAT("\033[%zuD", N);
    }

    // ANSI: Move cursor to specific position (X, Y)
    static string_t cursor_position(size_t X, size_t Y) {
        return FORMAT("\033[%zu;%zuH", Y, X);
    }

};


UI_NAMESPACE_END