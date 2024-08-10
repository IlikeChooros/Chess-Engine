#include "cengine/manager_impl.h"


namespace chess{

    // C'tor

    ManagerImpl::ManagerImpl(Board* board)
    {
        this->n_moves = 0;
        this->board = board;
        this->curr_move = Move();
        this->search_params = SearchParams();

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
        return *this;
    }

    ManagerImpl::~ManagerImpl()
    {
        stopSearchAsync();
    }

    /**
     * @brief Reloads the manager with a new board state, resetting all variables and generating moves
     */
    void ManagerImpl::reload()
    {
        this->curr_move = Move();
        this->n_moves = 0;
        this->move_list.clear();
        this->history.clear();
        (void)generateMoves();
        pushHistory();
    }
}