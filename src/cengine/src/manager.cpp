#include <cengine/manager.h>

namespace chess
{

    Manager::Manager(Board* board)
    {
        m_impl = std::make_unique<ManagerImpl>(board);
    }

    Manager& Manager::operator=(Manager&& other)
    {
        m_impl = std::move(other.m_impl);
        return *this;
    }

    /**
     * @brief Load fen string, with 'moves' support
     */
    void Manager::loadFen(std::string fen)
    {
        std::istringstream iss(fen);
        std::string base_fen;
        std::string section;
        int sections = 0;
        while(iss >> section){
            if (section == "moves")
                break;
            base_fen += section + " ";
            sections++;
        }

        if (sections < 6)
            return;

        impl()->board->loadFen(base_fen.c_str());
        reload();

        // Handle moves
        while(iss >> section){
            if (!makeMove(section)){
                break;
            }
        }
    }
    

    /**
     * @brief Moves a piece from one square to another, if the move is valid
     * it updates the board, calls move generation and changes the side to move
     * @param from The square to move the piece from (as an index)
     * @param to The square to move the piece to (as an index)
     * @param flags The flags for the move, default is 0
     * @return True if the move was successful, false otherwise
     */
    bool Manager::makeMove(uint32_t from, uint32_t to, int flags)
    {
        int* iboard = this->board()->getBoard();
        if(iboard[from] == Piece::Empty || from == to || Piece::getColor(iboard[from]) != this->board()->getSide())
            return false;
        
        if(flags == -1){
            auto vflags = getFlags(from, to);
            if(vflags.empty() || vflags.size() > 1) // If there are no valid flags or more than one (promotion)
                return false;
            flags = vflags[0]; // Set default flags
        }

        Move find(from, to, flags);
        for(int i = 0; i < m_impl->n_moves; i++){
            auto move = Move(m_impl->move_list[i]);
            if (move == find){
                m_impl->make(move);
                m_impl->generateMoves();
                return true;
            }
        }

        std::cout << "Invalid move: " << Move::notation(from, to) << std::endl;
        return false;
    }

    /**
     * @brief Make a move from a string, using format: square_from (ex. e4) + square_to (ex. f6) + promotion_piece ('n', 'b', 'r', 'q')
     * for example: e2e4, e7e8q
     */
    bool Manager::makeMove(std::string move)
    {
        Move m(move);

        if (!(m && m.setFlags(move, getFlags(m.getFrom(), m.getTo())))){
            return false;
        }

        return makeMove(m.getFrom(), m.getTo(), m.getFlags());
    }

    /**
     * @brief Get all possible moves for a piece at a given square
     */
    std::list<Manager::PieceMoveInfo> Manager::getPieceMoves(uint32_t from)
    {
        std::list<PieceMoveInfo> moves;
        for(int i = 0; i < m_impl->n_moves; i++){
            auto move = Move(m_impl->move_list[i]);
            if (move.getFrom() == from){
                moves.push_back({
                    move.getTo() % 8,
                    move.getTo() / 8,
                    move.getFlags()
                });
            }
        }
        return moves;
    }

    /**
     * @brief Check if a move is a promotion
     */
    bool Manager::isPromotion(uint32_t from, uint32_t to)
    {
        for(int i = 0; i < m_impl->n_moves; i++){
            auto move = Move(m_impl->move_list[i]);
            if ((move.getFrom() == from) && (move.getTo() == to)
                && (move.getFlags() & Move::FLAG_PROMOTION))
            {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Get the flags for a move
     */
    std::vector<uint32_t> Manager::getFlags(uint32_t from, uint32_t to)
    {
        std::vector<uint32_t> flags;
        flags.reserve(4);
        for(int i = 0; i < m_impl->n_moves; i++){
            auto move = Move(m_impl->move_list[i]);
            if (move.getFrom() == from && move.getTo() == to){
                flags.push_back(move.getFlags());
            }
        }
        return flags;
    }

    /**
     * @brief Get all the moves that can move to a square
     */
    MoveList Manager::canMoveTo(uint32_t to, int type)
    {
        int* iboard = this->board()->getBoard();
        MoveList ml;
        for(int i = 0; i < m_impl->n_moves; i++){
            auto move = Move(m_impl->move_list[i]);

            if (type != -1 && Piece::getType(iboard[move.getFrom()]) != type)
                continue;

            if (move.getTo() == to)
                ml.add(move);
        }
        return ml;
    }
}