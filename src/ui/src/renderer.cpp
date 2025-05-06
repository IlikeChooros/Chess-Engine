#include <ui/renderer.hpp>


UI_NAMESPACE_BEGIN

void Renderer::render_board(const chess::Board& board, bool side)
{
    using namespace chess;

    // --- Add this at the beginning ---
    // Option 1: Move cursor home (less flicker)
    std::cout << CURSOR_HOME;
    // Option 2: Clear screen and move cursor home
    // std::cout << CLEAR_SCREEN << CURSOR_HOME;
    // ---------------------------------

    int increment = side ? -1 : 1;
    int start = side ? 7 : 0;

    std::cout << (side ? "  a b c d e f g h  \n" : "  h g f e d c b a  \n"); // Added padding for potential overwrite issues ); // Added padding for potential overwrite issues
    std::cout << " +----------------+ \n";
    for (int i = start; (side) ? i >= 0 : i <= 7; i += increment)
    {
        renderLine(board, i, side, std::cout);
    }
    std::cout << " +----------------+ \n";
}

/**
 * @brief Render a single line (row) of the chess board
 */
void Renderer::renderLine(const chess::Board& board, int row, bool side, std::ostream& os)
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

/**
 * @brief Render engine search result with left padding. Prints
 * the depth, evaluation score, and best move
 */
void Renderer::render_engine_line(const chess::Result& result, bool pv)
{
    using namespace chess;
    print<false, false>('\r', M_make_spaces(max_dots_count + 3), FG_LIGHT_GRAY, ": depth ", result.depth, " eval ");

    // Format the score
    if (result.score.type == Score::mate)
    {
        // Mate score, print as "M" or "-M"
        print<false, false>(
            (result.score.value > 0 ? "M" : "-M"), 
            std::abs(result.score.value)
        );
    }
    else
    {
        // Centipawn score, print as a decimal value
        print<false, false>(
            " ", std::setprecision(2), 
            result.score.value / 100.0
        );
    }

    // Print the current best move
    if (!pv)
        print(" bestmove ", result.bestmove.uci());
    else
    {
        // Print the principal variation
        print<false, false>(" pv ");
        for(auto i = 0; i < result.pv.size(); i++) {
            print<false, false>(result.pv[i].uci(), ' ');
        }
    }
}

/**
 * @brief Render engine outputs after successful search.
 */
void Renderer::render_engine_outputs(const chess::Result& result, bool pv)
{
    if (result.bestmove == chess::Move::nullMove)
        return;

    // Print the final result with centered OK
    render_engine_line(result, pv);
    print<false, false>('\r', OK_BG, OK_FG, "[", M_make_spaces(max_dots_count), "]");

    // move the cursor to the center, print OK, and move down
    print<false>('\r', cursor_forward(max_dots_count / 2), "OK");
    print<false, false>('\r', cursor_down(1));

    // Print the engine move
    print(FG_LIGHT_GRAY, 
        "Engine move: ", result.bestmove.uci(), cursor_down(1)
    );
    flush();
}

UI_NAMESPACE_END

