#include "cengine/manager_impl.h"


namespace chess{

    // C'tor

    ManagerImpl::ManagerImpl(Board* board)
    {
        this->state = GameState::Normal;
        this->n_moves = 0;
        this->board = board;
        this->curr_move = Move();
        this->search_params = DEFAULT_SEARCH_PARAMS;

        if (board == nullptr)
            return;

        pushHistory();
    }

    ManagerImpl& ManagerImpl::operator=(ManagerImpl&& other)
    {
        this->board = other.board;
        this->n_moves = other.n_moves;
        this->move_list = std::move(other.move_list);
        this->curr_move = other.curr_move;
        this->history = other.history;
        this->state = other.state;
        return *this;
    }

    /**
     * @brief Reloads the manager with a new board state, resetting all variables and generating moves
     */
    void ManagerImpl::reload()
    {
        this->state = GameState::Normal;
        this->curr_move = Move();
        this->n_moves = 0;
        this->move_list.clear();
        this->history.clear();
        (void)generateMoves();
        pushHistory();
    }

    /**
     * @brief Evaluates the current game state, should be called after `generateMoves()`
     */
    ManagerImpl::GameState ManagerImpl::evalState(){

        // fifty-move rule
        if (board->halfmoveClock() >= 100){
            return GameState::Draw;
        }

        // If there are moves available
        if (n_moves != 0){
            // TODO:
            // Threefold repetition
            return GameState::Normal;
        }

        // There are no moves available, so check if the king is in check
        // bool is_white = board->getSide() == Piece::White;
        // uint64_t king_attack = attacks_to[!is_white][bitScanForward(board->bitboards(is_white)[Piece::King - 1])];
        
        // if (king_attack == 0){
        //     // Stalemate
        //     return GameState::Draw;
        // }
        return GameState::Checkmate;
    }
}