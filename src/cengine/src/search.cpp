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
        m_best_result  = Result{};
        m_interrupt    = Interrupt(limits);
        m_board        = board;
        m_search_cache = &search_cache;
        m_limits       = limits;
        m_ss.clear();
        m_root_pv.clear();
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
    Move Thread::get_pv_move(Depth& ply)
    {        
        // Extensions
        if (ply >= Depth(m_root_pv.size()))
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
        Value delta           = 50;
        m_depth               = 1;
        m_result              = {};
        m_root_age            = m_board.halfmoveClock();
        int whotomove         = m_board.turn() ? 1 : -1;

        // Ignoring the signal, so that I will always get pv from searching
        m_interrupt.set_ignore(); 

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
        while(m_depth < MAX_PLY && !m_interrupt.get())
        {
            // Aspiration window
            while(true)
            {
                eval = search<Root>(m_board, alpha, beta, m_depth, 0);

                if (eval <= alpha)
                {
                    alpha = std::max(alpha - delta, MIN);
                }
                else if (eval >= beta)
                {
                    beta = std::min(beta + delta, MAX);
                }
                else
                {
                    break;
                }

                delta += delta / 2;
            }
            
            // eval = search<Root>(m_board, alpha, beta, m_depth);
            if (abs(eval) >= MATE_THRESHOLD)
                m_result.pv       = get_pv(MAX_PLY);
            else
                m_result.pv       = get_pv(m_depth);
            
            // Save the pv
            m_root_pv         = m_result.pv;
            m_result.bestmove = m_result.pv[0];

            // Update alpha beta
            alpha  = eval - delta;
            beta   = eval + delta;

            if (m_interrupt.get())
            {
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

            // Update the interrupt ignore flag
            if (m_interrupt.is_ignoring())
                m_interrupt.restore_state(m_depth);

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
        MoveList moves  = board.generateLegalCaptures(); 
        Value     eval  = Eval::evaluate(board);

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
    Value Thread::search(Board& board, Value alpha, Value beta, Depth depth, Depth ply, bool)
    {
        constexpr bool isRoot       = nType == Root;
        constexpr NodeType nextType = isRoot ? PV : nType;
        // constexpr bool isPv         = isRoot || nextType == PV;

        bool null_window = beta == alpha + 1;

        // Update interrupt
        m_interrupt.update();
        
        // Step 1: Check if this node is terminated
        // Generate legal moves, setup variables for the search
        MoveList moves  = board.generateLegalMoves();
        Value best      = MATE - depth;

        // Look for draw conditions and check if the game is over
        if (board.isTerminated(&moves))
        {
            auto termination = board.getTermination();
            if (termination != Termination::CHECKMATE)
                best = 0;
            return best;
        }

        // Step 2:
        // Lookup transposition table and check for possible cutoffs
        Move hash_move = Move::nullMove;
        uint64_t  hash = board.getHash();
        int  old_alpha = alpha;
        
        if (m_search_cache->getTT().contains(hash))
        {
            TEntry entry = m_search_cache->getTT().get(hash);
            hash_move    = entry.bestMove;
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

        // Step 2a: Check extensions
        if (board.m_in_check)
            depth++;

        // Step 3: If depth reaches 0, do non-quiet move search
        // Quiescence search
        if (depth <= 0)
            return qsearch(board, alpha, beta, ply);
        
        Move  bestmove            = Move::nullMove;        
        // bool improving            = m_ss.improving(board, ply);
        // bool in_check             = board.m_in_check;
        auto static_eval          = Eval::evaluate(board);
        m_ss.get(ply).static_eval = static_eval;

        // Step 4 (NMP if in null window)
        // if (nmp && null_window && NMP::valid(depth, board))
        // {
        //     // Try NMP if static evaluation is higher than beta
        //     if (static_eval >= beta)
        //     {
        //         int R = NMP::reduce(depth);
        //         board.makeNullMove();
        //         Value eval = -search<nonPV>(board, -beta, -alpha, depth - R, ply + 1, false);
        //         board.undoNullMove();

        //         if (eval >= beta)
        //             return eval;
        //     }
        // }

        // Step 5:
        // Sort the moves using move ordering
        MoveOrdering::sort(&moves, get_pv_move(ply), &board, m_search_cache, ply);

        // Step 6:
        // Loop through the rest of the moves
        for (size_t i = 0; i < moves.size(); i++)
        {
            Move m = moves[i];
            Value eval = best;

            board.makeMove(m);
    
            // Step 6a:
            // LMR + PVS
            // By the move ordering, we assume that the 1st move is the PV.
            // So search with full window that move (as well as first 6 moves)
            // Then try null window search with reduced depth, and see if it fails high
            // If so, then do a research.
            if ((i >= 3))
            {
                // int r = 1;
                // if (LMR::valid(ply, m, in_check, improving, m_search_cache))
                    // r = LMR::reduce(depth, i, null_window);
                
                eval = -search<nonPV>(board, -alpha - 1, -alpha, depth - 1, ply + 1);

                // Zw search with depth reduction
                if (eval > alpha && !(null_window)
                    // If it fails high, check if not in null window, then try search with full depth, on zw,
                    // If THAT fails, then do a full research.
                    // && (!null_window && -search<nonPV>(board, -alpha - 1, -alpha, depth - 1, ply + 1) > alpha)
                )
                    eval = -search<PV>(board, -beta, -alpha, depth - 1, ply + 1);
            }
            else
            {
                // Do a full search
                eval = -search<nextType>(board, -beta, -alpha, depth - 1, ply + 1);
            }

            board.undoMove(m);

            if (m_interrupt.get())
                return 0;

            if (eval > best)
            {
                // Update the search best score, best move
                best           = eval;
                alpha          = std::max(alpha, best);
                bestmove       = m;

                if (best >= beta)
                    break; 
            }
        }

        // Step 7:
        // Store the best move in the transposition table
        TEntry entry;
        entry.hash      = hash;
        entry.depth     = depth;
        entry.score     = best;
        entry.bestMove  = bestmove;
        entry.age       = m_root_age;

        if (best <= old_alpha)
            entry.nodeType = TEntry::UPPERBOUND;
        else if (best >= beta)
        {
            entry.nodeType = TEntry::LOWERBOUND;
            
            // Beta-cutoff, add that to the history and update killers
            if (bestmove.isQuiet())
            {
                m_search_cache->getHH().update(board.turn(), bestmove, depth);
                m_search_cache->getKH().update(bestmove, ply);
            }
        }
        else
            entry.nodeType = TEntry::EXACT;
        
        m_search_cache->getTT().store(entry);

        return best;
    }
}