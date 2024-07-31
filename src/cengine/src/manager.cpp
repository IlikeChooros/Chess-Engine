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
     * @brief Moves a piece from one square to another, if the move is valid
     * it updates the board, calls move generation and changes the side to move
     * @param from The square to move the piece from (as an index)
     * @param to The square to move the piece to (as an index)
     * @param flags The flags for the move, default is 0
     * @return True if the move was successful, false otherwise
     */
    bool Manager::movePiece(uint32_t from, uint32_t to, int flags)
    {
        int* iboard = this->board()->board.get();
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

        dlogf("Invalid move: from %s to %s\n",
                square_to_str(from).c_str(),
                square_to_str(to).c_str()
        );
        return false;
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
                    .x = move.getTo() % 8,
                    .y = move.getTo() / 8,
                    .flags = move.getFlags()
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
}