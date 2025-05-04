#include <ui/board_renderer.hpp>


UI_NAMESPACE_BEGIN

// ANSI escape code to move cursor to top-left corner
const char* CURSOR_HOME = "\033[H";
// Optional: ANSI escape code to clear the entire screen
const char* CLEAR_SCREEN = "\033[2J";

void BoardRenderer::render(const chess::Board& board, bool side, std::ostream& os)
{
    using namespace chess;

    // --- Add this at the beginning ---
    // Option 1: Move cursor home (less flicker)
    os << CURSOR_HOME;
    // Option 2: Clear screen and move cursor home
    // os << CLEAR_SCREEN << CURSOR_HOME;
    // ---------------------------------

    int increment = side ? -1 : 1;
    int start = side ? 7 : 0;

    os << (side ? "  a b c d e f g h  \n" : "  h g f e d c b a  \n"); // Added padding for potential overwrite issues ); // Added padding for potential overwrite issues
    os << " +----------------+ \n";
    for (int i = start; (side) ? i >= 0 : i <= 7; i += increment)
    {
        renderLine(board, i, side, os);
    }
    os << " +----------------+ \n";
}

void BoardRenderer::renderLine(const chess::Board& board, int row, bool side, std::ostream& os)
{
    using namespace chess;

    // utf8 symbols for pieces
    const char* pieces[7] = {
        " ", "♟", "♞", "♚", "♝", "♜", "♛"
    };

    auto WHITE_SQUARE = Ansi::bg_rgb(172, 172, 172),
         BLACK_SQUARE = Ansi::bg_rgb(72, 72, 72);

    auto WHITE_PIECE = Ansi::fg_rgb(255, 255, 255),
         BLACK_PIECE = Ansi::fg_rgb(8, 8, 8);

    int increment = side ? 1 : -1;
    int start = side ? 0 : 7;

    os << row + 1 << "|";
    for (int j = start; (side) ? j <= 7 : j >= 0; j += increment)
    {
        int piece = board[(squares::getSquare(j, row))];

        // Set the background color based on the square color
        os << ((j + row) % 2 == 0 ? WHITE_SQUARE : BLACK_SQUARE);

        if (piece == Piece::Empty)
        {
            // Color the background based on the square color
            os << "  " << Ansi::RESET; // Empty square
        }
        else
        {
            bool color = Piece::isWhite(piece);
            int type = Piece::getType(piece);

            // Use the ANSI escape code for the background color
            os << (color ? WHITE_PIECE : BLACK_PIECE);

            // Use the piece symbol based on color and type
            os << pieces[type] << " " << Ansi::RESET;
        }
    }
    os << "| \n";
}

UI_NAMESPACE_END

