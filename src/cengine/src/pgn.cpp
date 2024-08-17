#include <cengine/pgn.h>


std::string PGN::get_move_notation(chess::Manager* m, chess::GameHistory *gh, Move move)
{
    using namespace chess;
    int* iboard = m->board()->getBoard();
    std::string notation = "";
    int piece = iboard[move.getFrom()];
    char pieceChar = Piece::toChar(piece);
    int type = Piece::getType(piece);
    bool sameFile = false;
    bool sameRank = false;

    if (move.isCastle()){
        if (move.isKingCastle())
            notation += "O-O";
        else
            notation += "O-O-O";
    } else {
        if (type != Piece::Pawn)
            notation += pieceChar;

        // Disambiguation
        auto moves = m->canMoveTo(move.getTo(), type);
        // Check if there are more than one piece that can move to the same square
        if (moves.size() > 1){
            for(auto i : moves){
                Move m(i);
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
    
    if (move.isCapture()){
        if (type == Piece::Pawn && !sameFile)
            notation += 'a' + move.getFrom() % 8;

        notation += "x";
    }
    
    m->makeMove(move.getFrom(), move.getTo(), move.getFlags());

    // Check if the move is a check
    auto status = m->getStatus();

    // target square
    if (!move.isCastle()){
        notation += square_to_str(move.getTo());
    }

    // Promotion
    if (move.isPromotion()){
        const char promotion_piece[4] = { 'N', 'B', 'R', 'Q' };
        notation += "=";
        notation += promotion_piece[move.getPromotionPiece()];
    }

    if (m->board()->inCheck()){
        if (status != GameStatus::CHECKMATE)
            notation += "+";
    }

    if (status == GameStatus::CHECKMATE)
        notation += "#";
    
    return notation + " ";
}

std::string PGN::pgn(chess::GameHistory* gh, PGNFields& fields)
{
    std::string pgn = "";

    pgn += "\n[Event \"" + fields.Event + "\"]\n";
    pgn += "[Site \"" + fields.Site + "\"]\n";
    time_t t = std::chrono::system_clock::to_time_t(fields.Date);
    char date[80];
    std::strftime(date, sizeof(date), "%Y.%m.%d", std::localtime(&t));
    pgn += "[Date \"" + std::string(date) + "\"]\n";
    pgn += "[Round \"" + std::to_string(fields.Round) + "\"]\n";
    pgn += "[White \"" + fields.White + "\"]\n";
    pgn += "[Black \"" + fields.Black + "\"]\n";
    pgn += "[Result \"" + pgn_game_status(fields.Result.status, fields.Result.colorWin == chess::Piece::White) + "\"]\n";

    if (fields.FEN != ""){
        pgn += "[FEN \"" + fields.FEN + "\"]\n";
        pgn += "[SetUp \"1\"]\n";
    }

    pgn += "\n";

    if (!gh || gh->history.size() <= 1)
        return pgn;
        
    // Move list, the first move is a nullmove, so we skip it
    auto it = gh->history.begin();
    it++;
    chess::Board board;
    if (fields.FEN != "")
        board.loadFen(fields.FEN);
    else
        board.init();
    chess::Manager manager(&board);
    manager.generateMoves();

    if (board.getSide() == chess::Piece::Black){
        pgn += std::to_string(board.fullmoveCounter()) + "... ";
        pgn += get_move_notation(&manager, gh, it->move);
        it++;
    }
    
    for(; it != gh->history.end();)
    {
        pgn += std::to_string(board.fullmoveCounter()) + ". ";
        pgn += get_move_notation(&manager, gh, it->move);
        it++;

        if (it == gh->history.end())
            break;

        pgn += get_move_notation(&manager, gh, it->move);
        it++;
    }

    pgn += pgn_game_status(fields.Result.status, fields.Result.colorWin);

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