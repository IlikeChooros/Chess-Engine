#include "cengine/manager_impl.h"


namespace chess{

    // C'tor

    ManagerImpl::ManagerImpl(Board* board)
    {
        this->state = GameState::Normal;
        this->captured_piece = Piece::Empty;
        this->n_moves = 0;
        this->move_list = std::vector<uint32_t>(256, 0);
        this->board = board;
        this->curr_move = Move();

        if (board == nullptr)
            return;

        pushHistory();
    }

    ManagerImpl& ManagerImpl::operator=(ManagerImpl&& other)
    {
        this->board = other.board;
        this->n_moves = other.n_moves;
        this->move_list = std::move(other.move_list);
        this->captured_piece = other.captured_piece;
        this->curr_move = other.curr_move;
        this->history = other.history;
        this->state = other.state;
        return *this;
    }

    /**
     * @brief Initializes the board bitboards, should be called once before generating moves
     */
    void ManagerImpl::init()
    {
        init_board(board);
    }

    /**
     * @brief Reloads the manager with a new board state, resetting all variables and generating moves
     */
    void ManagerImpl::reload()
    {
        this->state = GameState::Normal;
        this->captured_piece = Piece::Empty;
        this->curr_move = Move();
        this->n_moves = 0;
        this->move_list = std::vector<uint32_t>(256, 0);
        this->history.clear();
        (void)generateMoves();
        pushHistory();
    }

    /**
     * @brief Makes a move, updating the board, the side to move and the move history
     * @attention User should call `generateMoves()` after calling this function
     */
    void ManagerImpl::make(Move& move)
    {
        ::make(move, board, &history);
        curr_move = move;
    }

    /**
     * @brief Unmakes current move, restoring the board to the previous state, 
     * may be called only once after `make()`
     * @attention User should call `generateMoves()` after calling this function
     */
    void ManagerImpl::unmake()
    {
        ::unmake(curr_move, board, &history);
    }

    /**
     * @brief Pushes the current move to the history stack
     */
    void ManagerImpl::pushHistory(){
        history.push(board, curr_move);
    }

    /**
     * @brief Generate all legal moves for the current board state
     */
    int ManagerImpl::generateMoves(){
        MoveList ml;
        void(::gen_legal_moves(&ml, board));
        move_list = std::vector<uint32_t>(ml.begin(), ml.end());
        n_moves = ml.size();
        return n_moves;
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