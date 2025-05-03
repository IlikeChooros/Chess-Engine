#include <ui/board_renderer.hpp>


UI_NAMESPACE_BEGIN

void BoardRenderer::render(const chess::Board& board, std::ostream& os)
{
    using namespace chess;

    // utf8 symbols for pieces
    const char* pieceSymbols[2][7] = {
        { " ", "♙", "♘", "♔", "♗", "♖", "♕" }, // white pieces
        { " ", "♟", "♞", "♚", "♝", "♜", "♛" }  // black pieces
    };

    os << "  a b c d e f g h\n";
    os << "  ----------------\n";
    for (int i = 7; i >= 0; --i)
    {
        os << i + 1 << "|";
        for (int j = 0; j < 8; ++j)
        {
            int piece = board[i * 8 + j];
            
            if (piece == Piece::Empty)
            {
                // print either black or white square
                if ((i + j) % 2 == 0)
                    os << "░░";
                else
                    os << "▒▒";
            }
            else
            {
                bool color = Piece::isWhite(piece);
                int type = Piece::getType(piece);
                os << pieceSymbols[color][type] << " ";
            }
        }
        os << "\n";
    }
    os << "------------------\n";
}

UI_NAMESPACE_END


