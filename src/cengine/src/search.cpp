#include <cengine/search.h>

namespace chess
{
    // Check the search parameters and see if the search should stop
    bool keep_searching(SearchParams* params)
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
    int quiescence(Board* b, GameHistory* gh, SearchParams* params, int alpha, int beta)
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
    int negaAlphaBeta(Board* b, GameHistory* gh, SearchCache* sc, SearchParams* params, int alpha, int beta, int depth){
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

        order_moves(&ml, nullptr, b, sc);
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
    void search(Board* board, GameHistory* gh, SearchCache* sc, SearchParams* params, SearchResult* result)
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
            order_moves(&ml, &pv, board, sc);

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

        glogger.printf("bestmove %s\n", best_move.notation().c_str());
        glogger.logBoardInfo(board);
        glogger.logTTableInfo(&sc->getTT());

        // update `is_running` flag
        params->setSearchRunning(false);
    }
}