#include <cengine/search.h>

namespace chess
{
    constexpr int MIN = -(1 << 29),
                  MAX = 1 << 29,
                  MATE = -(1 << 16) + 1,
                  MATE_THRESHOLD = -MATE;

    static TimeManagement time_management = TimeManagement();
    static SearchCache *sc = nullptr;
    constexpr int ASP_WINDOW_SIZE = 25, ASP_WINDOW_WIDEN = 75;


    // Check the search parameters and see if the search should stop
    inline bool keep_searching(SearchParams* params){
        if (params->shouldStop())
            return false;

        if (time_management.end()){
            params->stopSearch();
            return false;
        }
        return true;
    }

    // Get the principal variation from the root
    MoveList getRootPV(Board* b, GameHistory *gh, int max_depth = 8) {
        if (!sc || !b || !gh)
            return {};

        MoveList pv;
        int depth = 0;
        uint64_t hash = b->hash();
        while (sc->getTT().contains(hash)) {
            TEntry entry = sc->getTT().get(hash);
            if (!entry.bestMove)
                break;
            if (pv.size() >= pv.capacity() - 1 || depth++ > max_depth)
                break;
            pv.add(entry.bestMove);
            ::make(entry.bestMove, b, gh);
            hash = b->hash();
        }
        for (auto it = pv.rbegin(); it != pv.rend(); it++) {
            ::unmake(Move(*it), b, gh);
        }
        return pv;
    }

    // Get PV from transposition table
    MoveList getPV(Board* b, GameHistory* gh, Move best_move, int max_depth = 10){
        if (!sc || !b || !gh || best_move == Move::nullMove)
            return {};

        int pv_depth = 0;
        MoveList pv;

        pv.add(best_move);
        ::make(best_move, b, gh);
        uint64_t hash = b->hash();
        while (sc->getTT().contains(hash)){
            TEntry entry = sc->getTT().get(hash);
            if (!entry.bestMove || entry.bestMove == best_move)
                break;
            if (pv.size() >= pv.capacity() - 1 || pv_depth++ > max_depth)
                break;
            pv.add(entry.bestMove);
            ::make(entry.bestMove, b, gh);
            hash = b->hash();
        }
        for (auto it = pv.rbegin(); it != pv.rend(); it++){
            ::unmake(Move(*it), b, gh);
        }
        return pv;
    }

    // Quiescence (no quiet) search, runs aplha beta on captures only and evaluates the position
    int quiescence(Board* b, GameHistory* gh, SearchParams* params, int alpha, int beta){
        int eval = evaluate(b);
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

    // Normal search with aspiration window
    int search(Board* b, GameHistory* gh, SearchParams* params, int alpha, int beta, int depth){

        // Lookup transposition table and check for possible cutoffs
        TTable<TEntry> *ttable = &sc->getTT();
        uint64_t hash = gh->back().hash;
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

        // Look for draw conditions and check if the game is over
        int best = MATE - depth;
        MoveList ml;
        void(::gen_legal_moves(&ml, b));
        auto status = get_status(b, gh, &ml);
        if (status != ONGOING){
            if (status == DRAW || status == STALEMATE)
                best = 0;
            return best;
        }

        // The game is not over, so do a non-quiet move search (captures only)
        if (depth == 0)
            return quiescence(b, gh, params, alpha, beta);

        // Do a principal variation search with aspiration window
        int last_irreversible = b->irreversibleIndex();
        Move best_move;
        order_moves(&ml, nullptr, b, sc);
        for (size_t i = 0; i < ml.size(); i++){
            Move m(ml[i]);
            ::make(m, b, gh);
            params->nodes_searched++;
            int eval = -search(b, gh, params, -beta, -alpha, depth -1);
            ::unmake(m, b, gh);
            b->irreversibleIndex() = last_irreversible;

            if (!keep_searching(params))
                break;

            if (eval > best){
                best = eval;
                best_move = ml[i];
            }
            
            if (eval > alpha)
                alpha = eval;

            if (alpha >= beta)
                break;
        }

        // Store the best move in the transposition table
        if (!params->shouldStop()){
            if (best <= old_alpha){
                // Alpha cutoff
                ttable->get(hash) = {hash, depth, TEntry::UPPERBOUND, best, best_move, gh->age()};
            }
            // A move is *too* good to be true
            else if (best >= beta){
                // Beta cutoff
                ttable->get(hash) = {hash, depth, TEntry::LOWERBOUND, best, best_move, gh->age()};
                sc->getHH().update(b->wside(), best_move, depth);
            }
            // Exact score of the position
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
    void search(Board* board, GameHistory* gh, SearchCache* search_cache, SearchParams* params, SearchResult* result)
    {
        
        if (!board || !gh || !search_cache || !result)
            return;

        {
            // Reset the search result
            std::lock_guard<std::mutex> lock(result->mutex);
            result->move = Move(0);
            result->depth = 0;
            result->status = ONGOING;
        }

        params->setSearchRunning(true);

        // Initialize variables
        sc = search_cache;
        int best_eval = MIN;
        Score nscore;
        Move best_move;
        MoveList pv = getRootPV(board, gh);
        MoveList ml;
        int last_irreversible = board->irreversibleIndex();
        void(::gen_legal_moves(&ml, board));
        int whotomove = board->getSide() == Piece::White ? 1 : -1;
        result->status = get_status(board, gh, &ml);

        if (result->status != ONGOING){
            glogger.printf("bestmove (none)\n");
            params->setSearchRunning(false);
            return;
        }

        // Prepare the timer
        time_management.reset(params->infinite, params->movetime);
        params->nodes_searched = 0;
        int depth = 0;
        int asp_window[2]{ MIN, MAX };

        // Iterative deepening
        while(true){
            int alpha = MIN, beta = MAX;
            order_moves(&ml, &pv, board, sc);

            // Loop through all the moves and evaluate them
            for (size_t i = 0; i < ml.size(); i++){
                Move m(ml[i]);
                ::make(m, board, gh);
                params->nodes_searched++;
                int eval = -search(board, gh, params, asp_window[0], asp_window[1], depth);
                if (alpha < eval && eval < beta && depth > 0){
                    eval = -search(board, gh, params, MIN, MAX, depth);
                    asp_window[0] = std::max(asp_window[0], eval);
                }
                ::unmake(m, board, gh);
                board->irreversibleIndex() = last_irreversible;

                if (!keep_searching(params))
                    break;

                if (eval > best_eval){
                    best_eval = eval;
                    best_move = ml[i];
                }

                if (alpha >= beta)
                    break;
            }
            
            // Update the aspiration window
            if (best_eval <= asp_window[0]){
                beta = best_eval + ASP_WINDOW_SIZE;
            }
            else if (best_eval >= asp_window[1]){
                alpha = best_eval - ASP_WINDOW_SIZE;
            }
            else {
                alpha = best_eval - ASP_WINDOW_WIDEN;
                beta = best_eval + ASP_WINDOW_WIDEN;
            }
            asp_window[0] = alpha;
            asp_window[1] = beta;
            depth++;
            pv = getPV(board, gh, best_move, depth + 1);

            // Update score
            nscore.value = best_eval * whotomove;
            if (abs(best_eval) >= MATE_THRESHOLD){
                nscore.value = (pv.size() + 1) / 2;
                nscore.value = std::max(nscore.value, 1);
                nscore.value *= best_eval > 0 ? 1 : -1;
                nscore.value *= whotomove;
                nscore.type = Score::mate;
            }

            // Update the search result 
            std::unique_lock<std::mutex> lock(result->mutex);
            if (lock.owns_lock()){
                result->move = best_move;
                result->score = nscore;
                result->time = time_management.elapsed();
                result->depth = depth - 1;
                result->pv = std::list<Move>(pv.begin(), pv.end());
                lock.unlock();
            }
            
            // Log the search info
            glogger.printInfo(
                depth, nscore.value, nscore.type == Score::cp, 
                params->nodes_searched, time_management.elapsed(), &pv, false
            );

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