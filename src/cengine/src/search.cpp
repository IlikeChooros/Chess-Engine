#include <cengine/search.h>

namespace chess
{

    Thread::Thread()
    {
        m_thinking = false;
    }

    Thread::~Thread()
    {
        stop();
    }

    /**
     * @brief Launch the search thread
     */
    void Thread::start_thinking(Board& board, SearchCache& search_cache, SearchLimits& limits)
    {
        Limits lim;
        lim.depth = limits.depth;
        lim.nodes = limits.nodes;
        lim.time.movetime = limits.movetime;
        lim.time.infinite = limits.infinite;

        m_promise      = std::promise<Result>();
        m_interrupt    = Interrupt(lim);
        m_board        = board;
        m_search_cache = &search_cache;
        m_limits       = limits;
        m_thread       = std::thread(&Thread::iterative_deepening, this);
    }

    /**
     * @brief Stop the search
     */
    void Thread::stop()
    {
        m_interrupt.stop();
        join();
    }

    /**
     * @brief Join the thread
     */
    void Thread::join()
    {
        if(m_thread.joinable())
            m_thread.join();
    }

    /**
     * @brief Get the principal variation from the transposition table
     */
    MoveList Thread::get_pv(int max_depth)
    {
        int pv_depth  = 0;
        MoveList pv   = {};
        uint64_t hash = m_board.hash();

        // Search through the transposition table for the principal variation
        while (m_search_cache->getTT().contains(hash))
        {
            TEntry entry = m_search_cache->getTT().get(hash);
            if (!entry.bestMove || pv.size() >= pv.capacity() - 1 || pv_depth++ > max_depth)
                break;
            
            // Add the move to the PV & make the move
            pv.add(entry.bestMove);
            m_board.makeMove(entry.bestMove);
            hash = m_board.hash();
        }

        // Unmake the moves
        for (auto it = pv.rbegin(); it != pv.rend(); it++)
        {
            m_board.undoMove(*it);
        }

        return pv;
    }

    /**
     * @brief Iterative deepening search
     * @warning Engine should check if the game is over before calling this function
     */
    void Thread::iterative_deepening()
    {
        m_thinking = true;

        // Initialize variables
        Value eval            = 0;
        Value besteval        = MIN;
        Value alpha           = MIN;
        Value beta            = MAX;
        int depth             = 1;
        m_result              = {};
        int whotomove         = m_board.turn() ? 1 : -1;

        // Iterative deepening loop
        while(true)
        {
            eval = search<Root>(m_board, alpha, beta, depth);

            // Check if the search should stop
            if (m_interrupt.get())
                break;

            // Update best evaluation & alpha
            if (eval > besteval)
            {
                besteval = eval;
                alpha = std::max(alpha, besteval);
            }

            if (eval >= beta)
                break;

            // Print info
            m_result.pv = get_pv(10);
            update_score(m_result.score, besteval, whotomove, m_result.pv); 
            glogger.printInfo(
                depth, m_result.score.value, m_result.score.type == Score::cp, 
                m_interrupt.nodes(), m_interrupt.time(), &m_result.pv
            );

            // Update depth & alpha beta
            depth += 1;
            m_interrupt.depth(depth);

            alpha = MIN;
            beta = MAX;
        }

        // Update promise
        m_promise.set_value(m_result);

        // Print the best move
        m_result.bestmove = m_result.pv.size() > 0 ? m_result.pv[0] : Move();
        glogger.printf("bestmove %s\n", m_result.bestmove.uci().c_str());
        glogger.logBoardInfo(&m_board);
        glogger.logTTableInfo(&m_search_cache->getTT());
    
        m_thinking = false;
    }

    /**
     * @brief Run quiescence search
     */
    Value Thread::qsearch(Board& board, Value alpha, Value beta, int depth)
    {   
        // Evaluate the position
        Value     eval = evaluate(&board, nullptr, nullptr);
        bool      turn = board.turn();
        MoveList moves = board.generateLegalCaptures();
        Value    best  = eval;
        // auto    status = get_status(&board, &moves);
        
        // if (status != ONGOING)
        // {
        //     if (status == DRAW || status == STALEMATE)
        //         return 0;
        //     return MATE;
        // }

        // Alpha beta pruning, if the evaluation is greater or equal to beta
        // that means the position is 'too good' for the side to move
        if (best >= beta)
            return best;

        // Update alpha & get the legal captures
        alpha = std::max(alpha, best);

        // Loop through all the captures and evaluate them
        for (size_t i = 0; i < moves.size(); i++)
        {
            m_interrupt.update(turn);
            Move m = moves[i];

            board.makeMove(m);
            eval = -qsearch(board, -beta, -alpha, depth - 1);
            board.undoMove(m);

            if (eval >= best)
            {
                best = eval;
                alpha = std::max(alpha, best);
            }

            if (best >= beta)
                break;
        }

        // Return the best possible evaluation
        return best;
    }

    /**
     * @brief Priciple variation search
     */
    template <NodeType nType>
    Value Thread::search(Board& board, Value alpha, Value beta, int depth)
    {
        constexpr bool isRoot = nType == Root;

        // Lookup transposition table and check for possible cutoffs
        uint64_t hash = board.hash();
        int old_alpha = alpha;
        
        if (m_search_cache->getTT().contains(hash))
        {
            TEntry entry = m_search_cache->getTT().get(hash);
            if (entry.depth >= depth)
            {
                if (entry.nodeType == TEntry::EXACT)
                    return entry.score;
                if (entry.nodeType == TEntry::LOWERBOUND)
                    alpha = std::max(alpha, entry.score);
                if (entry.nodeType == TEntry::UPPERBOUND)
                    beta = std::min(beta, entry.score);

                if (alpha >= beta)
                    return entry.score;
            }
        }

        // Check if the search should stop
        if (m_interrupt.get())
            return 0;

        // Quiescence search
        if (depth == 0)
            return qsearch(board, alpha, beta, depth);
        
        // Generate legal moves, setup variables for the search
        MoveList moves     = board.generateLegalMoves();
        Value best         = MIN;
        Move  bestmove     = Move::nullMove;
        bool  turn         = board.turn();

        // Look for draw conditions and check if the game is over
        auto status = get_status(&board, &moves);
        if (status != ONGOING)
        {
            if (status == DRAW || status == STALEMATE)
                best = 0;
            return best;
        }

        // Sort the moves using move ordering
        if constexpr (isRoot)
        {
            MoveList pv = get_pv(10);
            MoveOrdering::sort(&moves, &pv, &board, m_search_cache);
        }
        else
        {
            MoveOrdering::sort(&moves, nullptr, &board, m_search_cache);
        }
        
        // Loop through all the moves and evaluate them
        for (size_t i = 0; i < moves.size(); i++)
        {
            // Update interrupt
            m_interrupt.update(turn);

            Move m = moves[i];
            board.makeMove(m);
            Value eval = -search<nonRoot>(board, -beta, -alpha, depth - 1);
            board.undoMove(m);

            if (m_interrupt.get())
                return 0;

            if (eval > best)
            {
                best = eval;
                alpha = std::max(alpha, best);
                bestmove = m;
            }

            if (best >= beta)
                break;     
        }

        // Store the best move in the transposition table
        TEntry entry;
        entry.hash = hash;
        entry.depth = depth;
        entry.score = best;
        entry.bestMove = bestmove;

        if (best <= old_alpha)
            entry.nodeType = TEntry::UPPERBOUND;
        else if (best >= beta)
        {
            entry.nodeType = TEntry::LOWERBOUND;
            m_search_cache->getHH().update(turn, bestmove, depth);
        }
        else
            entry.nodeType = TEntry::EXACT;
        
        m_search_cache->getTT().store(hash, entry);

        return best;
    }


    // Check the search parameters and see if the search should stop
    bool keep_searching(SearchLimits* params)
    {
        using namespace std::chrono;

        if (params->shouldStop())
            return false;
        if (params->infinite)
            return true;
        if (duration_cast<milliseconds>(high_resolution_clock::now() - params->start_time).count() >= params->movetime){
            params->stopSearch();
            return false;
        }
        return true;
    }


    // Get PV from transposition table
    MoveList getPV(Board* b, SearchCache* sc, GameHistory* gh, Move best_move, int max_depth = 10)
    {
        if (!sc || !b || !gh || best_move == Move::nullMove)
            return {};

        int pv_depth = 0;
        MoveList pv;

        pv.add(best_move);
        ::make(best_move, b, gh);
        uint64_t hash = get_hash(b);

        while (sc->getTT().contains(hash))
        {
            TEntry entry = sc->getTT().get(hash);
            if (!entry.bestMove || entry.bestMove == best_move)
                break;
            if (pv.size() >= pv.capacity() - 1 || pv_depth++ > max_depth)
                break;
            pv.add(entry.bestMove);
            ::make(entry.bestMove, b, gh);
            hash = get_hash(b);
        }
        for (auto it = pv.rbegin(); it != pv.rend(); it++){
            ::unmake(Move(*it), b, gh);
        }
        return pv;
    }


    // Quiescence (no quiet) search, runs aplha beta on captures only and evaluates the position
    int quiescence(Board* b, GameHistory* gh, SearchLimits* params, int alpha, int beta)
    {
        // return evaluate(b, nullptr, nullptr);

        int eval = evaluate(b, nullptr, nullptr);
        if (eval >= beta)
            return beta;
        alpha = std::max(alpha, eval);
        
        MoveList ml;
        ::gen_captures(&ml, b);

        for (size_t i = 0; i < ml.size(); i++){
            Move m(ml[i]);
            ::make(m, b, gh);
            params->nodes_searched++;
            int score = -quiescence(b, gh, params, -beta, -alpha);
            ::unmake(m, b, gh);

            if (score >= beta)
                return beta;
            if (score > alpha)
                alpha = score;
        }
        return alpha;
    }

    // Negamax with alpha-beta pruning
    int negaAlphaBeta(Board* b, GameHistory* gh, SearchCache* sc, SearchLimits* params, int alpha, int beta, int depth){
        using namespace std::chrono;

        // Lookup transposition table and check for possible cutoffs
        TTable<TEntry> *ttable = &sc->getTT();
        uint64_t hash = get_hash(b);
        int old_alpha = alpha;
        if (ttable->contains(hash)){
            TEntry& entry = ttable->get(hash);
            if (entry.depth >= depth){
                if (entry.nodeType == TEntry::EXACT)
                    return entry.score;
                if (entry.nodeType == TEntry::LOWERBOUND)
                    alpha = std::max(alpha, entry.score);
                if (entry.nodeType == TEntry::UPPERBOUND)
                    beta = std::min(beta, entry.score);
                
                if (alpha >= beta)
                    return entry.score;
            }
        }

        if (depth == 0)
            return quiescence(b, gh, params, alpha, beta);

        MoveList ml;
        void(::gen_legal_moves(&ml, b));
        int best = MATE - depth;
        int last_irreversible = b->irreversibleIndex();
        Move best_move;

        // Look for draw conditions and check if the game is over
        auto status = get_status(b, gh, &ml);
        if (status != ONGOING){
            if (status == DRAW || status == STALEMATE)
                best = 0;
            return best;
        }

        MoveOrdering::sort(&ml, nullptr, b, sc);
        for (size_t i = 0; i < ml.size(); i++){
            Move m(ml[i]);
            ::make(m, b, gh);
            params->nodes_searched++;
            best = std::max(best, -negaAlphaBeta(b, gh, sc, params, -beta, -alpha, depth - 1));
            ::unmake(m, b, gh);
            b->irreversibleIndex() = last_irreversible;

            if (best > alpha){
                alpha = best;
                best_move = ml[i];
            }

            if (alpha >= beta){      
                break;
            }

            if (!keep_searching(params))
                break;
        }

        // Store the best move in the transposition table
        if (!params->shouldStop()){
            if (best <= old_alpha){
                // Alpha cutoff
                ttable->get(hash) = {hash, depth, TEntry::UPPERBOUND, best, best_move, gh->age()};
            }
            else if (best >= beta){
                // Beta cutoff
                ttable->get(hash) = {hash, depth, TEntry::LOWERBOUND, best, best_move, gh->age()};
                sc->getHH().update(b->getSide() == Piece::White, best_move, depth);
            }
            else {
                // Exact score
                ttable->get(hash) = {hash, depth, TEntry::EXACT, best, best_move, gh->age()};
            }
        }
            
        return best;
    }


    /**
     * @brief Search for the best move using the alpha-beta pruning algorithm,
     * thread unsafe only if GameHistory is shared between threads (board is copied)
     * 
     * TODO:
     * - [x] Implement iterative deepening
     * - [x] Implement quiescence search
     * - [x] Add time management
     * - Enhancements:
     *  - [x] Move ordering
     *  - [ ] SEE
     */
    void search(Board* board, GameHistory* gh, SearchCache* sc, SearchLimits* params, SearchResult* result)
    {
        
        if (!board || !gh || !sc || !result)
            return;

        {
            std::lock_guard<std::mutex> lock(result->mutex);
            result->move = Move(0);
            result->depth = 0;      
            result->status = ONGOING;       
        }

        params->setSearchRunning(true);

        // Initialize variables
        int best_eval = MIN;
        Score nscore;
        Move best_move;
        MoveList pv;
        MoveList ml;
        int last_irreversible = board->irreversibleIndex();
        void(::gen_legal_moves(&ml, board));
        int whotomove = board->getSide() == Piece::White ? 1 : -1;
        result->status = get_status(board, gh, &ml);

        if (result->status != ONGOING){
            params->setSearchRunning(false);
            return;
        }

        // Prepare the timer
        using namespace std::chrono;
        params->start_time = high_resolution_clock::now();
        params->nodes_searched = 0;
        int depth = 1;
        uint64_t time_taken = 1;

        // Iterative deepening
        while(true){
            int alpha = MIN, beta = MAX;
            MoveOrdering::sort(&ml, &pv, board, sc);

            // Loop through all the moves and evaluate them
            for (size_t i = 0; i < ml.size(); i++){
                Move m(ml[i]);
                ::make(m, board, gh);
                params->nodes_searched++;
                int eval = -negaAlphaBeta(board, gh, sc, params, alpha, beta, depth);
                ::unmake(m, board, gh);
                board->irreversibleIndex() = last_irreversible;

                if (eval > best_eval){
                    best_eval = eval;
                    best_move = ml[i];
                }

                if (!keep_searching(params))
                    break;

                alpha = std::max(alpha, best_eval);
                if (alpha >= beta)
                    break;
            }
            
            depth++;
            time_taken = duration_cast<milliseconds>(high_resolution_clock::now() - params->start_time).count();
            pv = getPV(board, sc, gh, best_move, depth);

            // Update score
            update_score(nscore, best_eval, whotomove, pv);

            // Update the search result 
            update_result(result, best_move, nscore, depth, time_taken, ONGOING, pv);
            
            // Log the search info
            glogger.printInfo(depth - 1, nscore.value, nscore.type == Score::cp, params->nodes_searched, time_taken, &pv);

            // Break if the search should stop
            if (depth > params->depth){
                params->stopSearch();
            }
            if (!keep_searching(params))
                break;
        }

        glogger.printf("bestmove %s\n", best_move.uci().c_str());
        glogger.logBoardInfo(board);
        glogger.logTTableInfo(&sc->getTT());

        // update `is_running` flag
        params->setSearchRunning(false);
    }
}