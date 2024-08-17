#include <cengine/search.h>

namespace chess
{
    constexpr int MIN = -(1 << 29),
                  MAX = 1 << 29,
                  MATE = -(1 << 28) + 1,
                  MATE_THRESHOLD = MATE - 1000;


    // Check the search parameters and see if the search should stop
    bool keep_searching(SearchParams* params){
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
    MoveList getPV(Board* b, SearchCache* sc, GameHistory* gh, Move best_move){
        if (!sc || !b || !gh || best_move == Move::nullMove)
            return {};

        MoveList pv;

        ::make(best_move, b, gh);
        uint64_t hash = get_hash(b);
        while (sc->getTT().contains(hash)){
            TEntry entry = sc->getTT().get(hash);
            if (!entry.bestMove || entry.bestMove == best_move)
                break;
            if (pv.size() >= pv.capacity() - 1)
                break;
            pv.add(entry.bestMove);
            ::make(entry.bestMove, b, gh);
            hash = get_hash(b);
        }
        for (auto it = pv.rbegin(); it != pv.rend(); it++){
            ::unmake(Move(*it), b, gh);
        }
        ::unmake(best_move, b, gh);
        return pv;
    }


    // Quiescence (no quiet) search, runs aplha beta on captures only and evaluates the position
    int quiescence(Board* b, GameHistory* gh, SearchParams* params, int alpha, int beta){
        // return evaluate(b, nullptr, nullptr);

        int eval = evaluate(b, nullptr, nullptr);
        if (eval >= beta)
            return beta;
        alpha = std::max(alpha, eval);
        
        MoveList ml;
        CacheMoveGen cache;
        ::gen_captures(&ml, b, &cache);

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
        
        CacheMoveGen cache;
        MoveList ml;
        void(::gen_legal_moves(&ml, b, &cache));
        int best = MATE - depth;
        int last_irreversible = b->irreversibleIndex();
        Move best_move;

        // Look for draw conditions and check if the game is over
        auto status = get_status(b, gh, &ml, &cache);
        if (status != ONGOING){
            if (status == DRAW || status == STALEMATE)
                best = 0;
            return best;
        }

        order_moves(&ml, b, &cache, sc);
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

        result->move = Move(0);
        result->depth = 0;
        result->score = 0;
        result->status = ONGOING;

        params->setSearchRunning(true);

        // Initialize variables
        int eval = MIN;
        Move best_move;
        MoveList ml;
        CacheMoveGen cache;
        int last_irreversible = board->irreversibleIndex();
        void(::gen_legal_moves(&ml, board, &cache));
        int whotomove = board->getSide() == Piece::White ? 1 : -1;
        result->status = get_status(board, gh, &ml, &cache);

        if (result->status != ONGOING){
            std::cout << "info string Game over\n";
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
            order_moves(&ml, board, &cache, sc);

            // Loop through all the moves and evaluate them
            for (size_t i = 0; i < ml.size(); i++){
                Move m(ml[i]);
                ::make(m, board, gh);
                params->nodes_searched++;
                int score = -negaAlphaBeta(board, gh, sc, params, alpha, beta, depth);
                ::unmake(m, board, gh);
                board->irreversibleIndex() = last_irreversible;

                if (score > eval){
                    eval = score;
                    best_move = ml[i];
                }

                if (!keep_searching(params))
                    break;

                alpha = std::max(alpha, eval);
                if (alpha >= beta)
                    break;
            }
            
            depth++;
            time_taken = duration_cast<milliseconds>(high_resolution_clock::now() - params->start_time).count();

            std::unique_lock<std::mutex> lock(result->mutex);
            if (lock.owns_lock()){
                result->move = best_move;
                result->score = eval * whotomove;
                result->time = time_taken;
                result->depth = depth;
                auto pv = getPV(board, sc, gh, best_move);
                result->pv = std::list<Move>(pv.begin(), pv.end());
                lock.unlock();
            }            

            if (params->depth != -1 && depth > params->depth){
                params->stopSearch();
            }
            if (!keep_searching(params))
                break;
        }

        depth--;
        time_taken = duration_cast<milliseconds>(high_resolution_clock::now() - params->start_time).count();
        time_taken = std::max(time_taken, 1UL);


        glogger.print("*** info bestmove %s eval %.2f \n", 
            Piece::notation(best_move.getFrom(), best_move.getTo()).c_str(),
            float(eval * whotomove) / 100.0f
        );
        auto pv = getPV(board, sc, gh, best_move);
        glogger.printPV(&pv);
        glogger.printStats(best_move, depth, eval * whotomove, params->nodes_searched, time_taken);
        glogger.print("\n");

        // update `is_running` flag
        params->setSearchRunning(false);
    }
}