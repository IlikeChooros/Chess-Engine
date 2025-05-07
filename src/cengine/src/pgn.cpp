#include <cengine/pgn.h>

// PGN Fields
const char* PGN::FIELDS_ORDER[] = {
    "Event", "Site", "Date", "Round", "White", "Black", "Result", "FEN", "SetUp"
};

/**
 * @brief Get the move notation for a move
 */
std::string PGN::get_move_notation(chess::Board& board, chess::Move move)
{
    using namespace chess;

    std::string notation;
    notation.reserve(10);
    int piece            = board[move.getFrom()];
    char pieceChar       = Piece::toChar(piece);
    int type             = Piece::getType(piece);
    bool sameFile        = false;
    bool sameRank        = false;

    if (move.isCastle())
    {
        if (move.isKingCastle())
            notation += "O-O";
        else
            notation += "O-O-O";
    }
    else 
    {
        if (type != Piece::Pawn)
            notation += pieceChar;

        // Disambiguation
        auto moves = board.filterMoves([&move, &board, &type](Move m){
            return m.getTo() == move.getTo() && Piece::getType(board[m.getFrom()]) == type;
        });

        // Check if there are more than one piece that can move to the same square
        if (moves.size() > 1){
            for(auto i : moves)
            {
                chess::Move m(i);
                if (m.getFrom() == move.getFrom())
                    continue;

                sameFile |= (m.getFrom() / 8) == (move.getFrom() / 8);
                sameRank |= (m.getFrom() % 8) == (move.getFrom() % 8);
            }

            if (sameFile)
                notation += 'a' + move.getFrom() % 8;
            if (sameRank)
                notation += '8' - move.getFrom() / 8;

            if (!sameFile && !sameRank){
                notation += 'a' + move.getFrom() % 8;
            }
        }
    }
    
    if (move.isCapture())
    {
        if (type == Piece::Pawn && !sameFile)
            notation += 'a' + move.getFrom() % 8;

        notation += "x";
    }
    
    board.makeMove(move);

    // target square
    if (!move.isCastle())
    {
        notation += square_to_str(move.getTo());
    }

    // Promotion
    if (move.isPromotion())
    {
        notation += "=";
        notation += Piece::toChar(Piece::promotionPieces[move.getPromotionPiece()]);
    }

    if (board.isTerminated())
    {
        auto termination = board.getTermination();
        if (termination == Termination::CHECKMATE)
            notation += "#";
    }
    else if (board.inCheck())
    {
        notation += "+";
    }
    
    return notation;
}

/**
 * @brief Generate the fields from the board
 */
void PGN::generate_fields(chess::Board board)
{
    board.isTerminated();
    fields["Event"]  = "A game";
    fields["Site"]   = "Somewhere";
    fields["Date"]   = std::chrono::system_clock::now();
    fields["Round"]  = 1;
    fields["White"]  = "Player 1";
    fields["Black"]  = "Player 2";
    fields["Result"] = pgn_game_status(board.getTermination(), !board.turn());
    fields["FEN"]    = "";
    fields["SetUp"]  = "";


    // Check if the board was setup 
    // (meaning if we undo all the moves, we do not get the starting position)
    auto history = board.history();
    chess::Board copy = board;
    for (auto it = history.rbegin(); it != history.rend(); it++)
    {
        copy.undoMove(it->move);
    }

    // If the board was setup, add the FEN
    if (copy.fen() != chess::Board::START_FEN)
    {
        fields["FEN"] = copy.fen();
        fields["SetUp"] = "1";
    }
}

/**
 * @brief Generate the PGN string from the game history
 */
std::string PGN::pgn(chess::Board board)
{
    std::string pgn = "";

    // Get the fields
    pgn = std::string(*this);

    pgn += "\n";
    
    chess::StateList& history = board.history();
    chess::Board copy;

    if (history.size() <= 1)
        return pgn;
    
    // Setup the copy
    if (fields["FEN"] != "")
        copy.loadFen(fields["FEN"].value);
    else
        copy.init();

    auto it = history.begin();
    it++; // Null move

    // If the game started with black (the position was setup and black is to move)
    if (board.getSide() == chess::Piece::Black)
    {
        pgn += std::to_string(copy.fullmoveCounter()) + "... "; // skip for white
        pgn += get_move_notation(copy, it->move) + " "; // black move
        it++;
    }
    
    // Loop through the history and generate the PGN, starting with white
    for(; it != history.end();)
    {
        // White move
        pgn += std::to_string(it->fullmove_counter) + ". ";
        pgn += get_move_notation(copy, it->move) + " ";
        it++;

        if (it == history.end())
            break;

        // Black move
        pgn += get_move_notation(copy, it->move) + " ";
        it++;
    }

    // Add the game result
    pgn += fields["Result"].value;

    return pgn;
}

std::string PGN::pgn_game_status(chess::Termination status, bool white)
{

    switch (status)
    {
    // No termination
    case chess::Termination::NONE:
        return "*";
    
    // Win conditions
    case chess::Termination::CHECKMATE:
    case chess::Termination::TIME:
    case chess::Termination::RESIGNATION:
        return white ? "1-0" : "0-1";

    // Draw conditions
    case chess::Termination::STALEMATE:
    case chess::Termination::DRAW:
    case chess::Termination::FIFTY_MOVES:
    case chess::Termination::THREEFOLD_REPETITION:
    case chess::Termination::FIVEFOLD_REPETITION:
    case chess::Termination::SEVENTYFIVE_MOVES:
    default:
        return "1/2-1/2";
    }
}