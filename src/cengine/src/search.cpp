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
     * @brief Setup the search parameters, prepare for iterative deepening
     */
    void Thread::setup(Board& board, SearchCache& search_cache, Limits& limits)
    {
        m_bestmove     = Move();
        m_best_result  = Result{};
        m_interrupt    = Interrupt(limits);
        m_board        = board;
        m_search_cache = &search_cache;
        m_limits       = limits;
    }

    /**
     * @brief Launch the search thread
     */
    void Thread::start_thinking(Board& board, SearchCache& search_cache, Limits& limits)
    {
        if (m_thinking)
            stop();
        
        setup(board, search_cache, limits);
        m_thread = std::thread(&Thread::iterative_deepening, this);
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
     * @brief Get the principal variation move at a certain depth of the search,
     * based on `m_root_pv`, should be already set.
     */
    Move Thread::get_pv_move(int depth)
    {
        Depth ply = m_depth - depth;
        
        if (ply < 0 || ply >= int(m_root_pv.size()))
            return Move();

        return m_root_pv.moves[ply];
    }

    /**
     * @brief Get the principal variation from the transposition table
     */
    MoveList Thread::get_pv(int max_depth)
    {
        int pv_depth  = 0;
        MoveList pv   = {};
        uint64_t hash = m_board.getHash();

        // Search through the transposition table for the principal variation
        while (m_search_cache->getTT().contains(hash))
        {
            TEntry entry = m_search_cache->getTT().get(hash);
            if (!entry.bestMove || pv.size() >= pv.capacity() - 1 || pv_depth++ > max_depth)
                break;
            
            // Add the move to the PV & make the move
            pv.add(entry.bestMove);
            m_board.makeMove(entry.bestMove);
            hash = m_board.getHash();
        }

        // Unmake the moves
        for (auto it = pv.rbegin(); it != pv.rend(); it++)
        {
            m_board.undoMove((*it));
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
        Value alpha           = MIN;
        Value beta            = MAX;
        // Value delta           = 50;
        m_depth               = 1;
        m_result              = {};
        int whotomove         = m_board.turn() ? 1 : -1;

        // Check if the game is over
        if (m_board.isTerminated())
        {
            m_result.status  = m_board.getTermination();
            m_best_result    = m_result;
            m_thinking       = false;
            glogger.printf("bestmove (none)\n");
            return;
        }

        // Iterative deepening loop
        while(m_depth <= MAX_PLY && !m_interrupt.get())
        {
            // Aspiration window
            // while(true)
            // {
            //     eval = search<Root>(m_board, alpha, beta, m_depth);

            //     if (eval <= alpha)
            //     {
            //         alpha = std::max(alpha - delta, MIN);
            //     }
            //     else if (eval >= beta)
            //     {
            //         beta = std::min(beta + delta, MAX);
            //     }
            //     else
            //     {
            //         break;
            //     }

            //     delta += delta / 2;
            // }
            
            eval              = search<Root>(m_board, alpha, beta, m_depth);
            if (abs(eval) >= MATE_THRESHOLD)
                m_result.pv       = get_pv(MAX_PLY);
            else
                m_result.pv       = get_pv(m_depth);
            
            // Save the pv
            m_root_pv         = m_result.pv;
            m_result.bestmove = m_result.pv.size() > 0 ? m_result.pv[0] : m_bestmove;

            // Update alpha beta
            // alpha    = eval - delta;
            // beta     = eval + delta;

            if (m_interrupt.get())
            {
                if (m_result.pv.empty())
                {
                    update_score(m_result.score, eval, whotomove);
                    m_best_result = m_result;
                }
                break;
            }

            update_score(m_result.score, eval, whotomove, m_result.pv.size());
            m_best_result = m_result;

            // Print info
            glogger.printInfo(
                m_depth, m_result.score.value, m_result.score.type == Score::cp, 
                m_interrupt.nodes(), m_interrupt.time(), &m_result.pv
            );

            // Check if mate has been found
            if (m_result.score.type == Score::mate && m_depth > 3)
                break;

            // Update depth & alpha beta
            m_depth += 1;
            m_interrupt.depth(m_depth);
        }


        if (m_depth > MAX_PLY)
        {
            glogger.logf("Max depth reached: %d\n", MAX_PLY);
            glogger.logf("position %s\n", m_board.fen().c_str());
        }

        // Print the best move
        glogger.printf("bestmove %s\n", m_result.bestmove.uci().c_str());
        glogger.logBoardInfo(&m_board);
        glogger.logTTableInfo(&m_search_cache->getTT());
    
        m_thinking = false;
    }

    /**
     * @brief Run quiescence search
     */
    Value Thread::qsearch(Board& board, Value alpha, Value beta, Depth ply = 0)
    {   
        // Evaluate the position
        Value     eval  = Eval::evaluate(board);
        MoveList moves  = board.generateLegalCaptures();        

        // Alpha beta pruning, if the evaluation is greater or equal to beta
        // that means the position is 'too good' for the side to move
        if (eval >= beta)
            return beta;

        // Update alpha & get the legal captures
        alpha = std::max(alpha, eval);

        // Loop through all the captures and evaluate them
        for (size_t i = 0; i < moves.size(); i++)
        {
            m_interrupt.update();
            Move m = moves[i];

            board.makeMove(m);
            eval = -qsearch(board, -beta, -alpha, ply + 1);
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
    Value Thread::search(Board& board, Value alpha, Value beta, Depth depth, Depth extension)
    {
        constexpr bool isRoot       = nType == Root;
        constexpr bool pv           = nType == PV;
        constexpr NodeType nextType = isRoot ? PV : (pv ? PV : nonPV);

        // Update interrupt
        m_interrupt.update();

        // Look for draw conditions and check if the game is over
        MoveList moves  = board.generateLegalMoves();
        Value best      = MATE - depth;

        if (board.isTerminated(&moves))
        {
            auto termination = board.getTermination();
            if (termination != Termination::CHECKMATE)
                best = 0;
            return best;
        }

        // Lookup transposition table and check for possible cutoffs
        uint64_t hash = board.getHash();
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
            return qsearch(board, alpha, beta);
        
        // Generate legal moves, setup variables for the search
        Move  bestmove = Move::nullMove;

        // Sort the moves using move ordering
        MoveOrdering::sort(&moves, get_pv_move(depth), &board, m_search_cache);        
        
        // Loop through the rest of the moves
        for (size_t i = 0; i < moves.size(); i++)
        {
            Move m = moves[i];
            board.makeMove(m);

            // Add extensions
            // int ext    = Extensions::check(board, extension);
            int ext    = 0;
            Value eval = 0;
            eval       = -search<nextType>(board, -beta, -alpha, depth - 1 + ext, extension);

            board.undoMove(m);

            if (m_interrupt.get() && m_bestmove)
                return 0;

            if (eval > best)
            {
                best = eval;
                alpha = std::max(alpha, best);
                bestmove = m;

                // Store the best move, since pv may not be available yet
                if constexpr (isRoot)
                {
                    m_bestmove = bestmove;
                };
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
            m_search_cache->getHH().update(board.turn(), bestmove, depth);
        }
        else
            entry.nodeType = TEntry::EXACT;
        
        m_search_cache->getTT().store(hash, entry);

        return best;
    }
}