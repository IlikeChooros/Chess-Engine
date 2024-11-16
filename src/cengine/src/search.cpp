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
    void Thread::start_thinking(Board& board, SearchCache& search_cache, Limits& limits)
    {
        m_best_result  = Result{};
        m_interrupt    = Interrupt(limits);
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
        auto status           = get_status(m_board);

        // Check if the game is over
        if (status != ONGOING)
        {
            m_result.status  = status;
            m_best_result    = m_result;
            m_thinking       = false;
            glogger.printf("bestmove (none)\n");
            return;
        }

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
            m_result.pv = get_pv(16);
            update_score(m_result.score, besteval, whotomove, m_result.pv); 
            glogger.printInfo(
                depth, m_result.score.value, m_result.score.type == Score::cp, 
                m_interrupt.nodes(), m_interrupt.time(), &m_result.pv
            );

            // Update the result
            m_best_result = m_result;

            // Update depth & alpha beta
            depth += 1;
            m_interrupt.depth(depth);

            alpha = MIN;
            beta = MAX;
        }

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
        Value     eval  = evaluate(board);
        bool      turn  = board.turn();
        MoveList moves  = board.generateLegalCaptures();

        // Check if the search should stop
        // auto status = get_status(&board, &moves);
        // if (status != ONGOING)
        // {
        //     if (status == DRAW || status == STALEMATE)
        //         best = 0;
        //     return best;
        // }

        // (void)moves.filter(Move::capture);

        // Alpha beta pruning, if the evaluation is greater or equal to beta
        // that means the position is 'too good' for the side to move
        if (eval >= beta)
            return beta;

        // Update alpha & get the legal captures
        alpha = std::max(alpha, eval);

        // Loop through all the captures and evaluate them
        for (size_t i = 0; i < moves.size(); i++)
        {
            m_interrupt.update(turn);
            Move m = moves[i];

            board.makeMove(m);
            eval = -qsearch(board, -beta, -alpha, depth - 1);
            board.undoMove(m);

            if (alpha >= beta)
                return beta;

            alpha = std::max(alpha, eval);
        }

        // Return the best possible evaluation
        return alpha;
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
        Value best         = MATE - depth;
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
        entry.hash      = hash;
        entry.depth     = depth;
        entry.score     = best;
        entry.bestMove  = bestmove;

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
}