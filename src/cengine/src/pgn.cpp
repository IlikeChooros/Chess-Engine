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

    // Check if the move is a check
    auto status = get_status(board);

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

    if (board.inCheck()){
        if (status != GameStatus::CHECKMATE)
            notation += "+";
    }

    if (status == GameStatus::CHECKMATE)
        notation += "#";
    
    return notation;
}

/**
 * @brief Generate the fields from the board
 */
void PGN::generate_fields(chess::Board board)
{

    fields["Event"]  = "A game";
    fields["Site"]   = "Somewhere";
    fields["Date"]   = std::chrono::system_clock::now();
    fields["Round"]  = 1;
    fields["White"]  = "Player 1";
    fields["Black"]  = "Player 2";
    fields["Result"] = pgn_game_status(chess::get_status(board), !board.turn());
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

    if (history.size() == 0)
        return pgn;
    
    // Setup the copy
    if (fields["FEN"] != "")
        copy.loadFen(fields["FEN"]);
    else
        copy.init();

    auto it = history.begin();

    // If the game started with black (the position was setup and black is to move)
    if (board.getSide() == chess::Piece::Black)
    {
        pgn += std::to_string(copy.fullmoveCounter()) + "... "; // skip for white
        pgn += get_move_notation(copy, it->move); // black move
        it++;
    }
    
    // Loop through the history and generate the PGN, starting with white
    for(; it != history.end();)
    {
        // White move
        pgn += std::to_string(copy.fullmoveCounter()) + ". ";
        pgn += get_move_notation(copy, it->move) + " ";
        it++;

        if (it == history.end())
            break;

        // Black move
        pgn += get_move_notation(copy, it->move) + " ";
        it++;
    }

    // Add the game result
    pgn += fields["Result"];

    return pgn;
}

std::string PGN::pgn_game_status(chess::GameStatus status, bool white)
{
    switch (status)
    {
    case chess::GameStatus::CHECKMATE:
        return white ? "1-0" : "0-1";
    case chess::GameStatus::STALEMATE:
        return "1/2-1/2";
    case chess::GameStatus::DRAW:
        return "1/2-1/2";
    default:
        return "*";
    }
}