#include <cengine/search.h>

namespace chess
{
    constexpr int MIN = -(1 << 29),
                  MAX = 1 << 29,
                  MATE = -(1 << 28) + 1;


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


    // Quiescence (no quiet) search, runs aplha beta on captures only and evaluates the position
    int quiescence(Board* b, GameHistory* gh, SearchParams* params, int alpha, int beta){
        return evaluate(b, nullptr, nullptr);

        // int eval = evaluate(b, nullptr, nullptr);
        // if (eval >= beta)
        //     return beta;
        // alpha = std::max(alpha, eval);
        
        // MoveList ml;
        // ::gen_captures(&ml, b);

        // for (size_t i = 0; i < ml.size(); i++){
        //     Move m(ml[i]);
        //     ::make(m, b, gh);
        //     params->nodes_searched++;
        //     int score = -quiescence(b, gh, params, -beta, -alpha);
        //     ::unmake(m, b, gh);

        //     if (score >= beta)
        //         return beta;
        //     if (score > alpha)
        //         alpha = score;
        // }
        // return alpha;
    }

    // Negamax with alpha-beta pruning
    int negaAlphaBeta(Board* b, GameHistory* gh, SearchCache* sc, SearchParams* params, int alpha, int beta, int depth){
        using namespace std::chrono;
        
        if (depth == 0)
            return quiescence(b, gh, params, alpha, beta);

        CacheMoveGen cache;
        MoveList ml;
        void(::gen_legal_moves(&ml, b, &cache));
        order_moves(&ml, b, &cache, sc);
        int best = MATE - depth;
        int last_irreversible = b->irreversibleIndex();
        TTable<TEntry> *ttable = &sc->getTT();

        // Look for draw conditions and check if the game is over
        auto status = get_status(b, gh, &ml, &cache);
        if (status != ONGOING){
            if (status == DRAW || status == STALEMATE)
                best = 0;
            return best;
        }

        // Lookup transposition table and check for possible cutoffs
        uint64_t hash = get_hash(b);
        int old_alpha = alpha;
        if (ttable->contains(hash)){
            TEntry entry = ttable->get(hash);
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

        for (size_t i = 0; i < ml.size(); i++){
            Move m(ml[i]);
            ::make(m, b, gh);
            params->nodes_searched++;
            best = std::max(best, -negaAlphaBeta(b, gh, sc, params, -beta, -alpha, depth - 1));
            ::unmake(m, b, gh);
            b->irreversibleIndex() = last_irreversible;

            alpha = std::max(alpha, best);
            if (alpha >= beta){
                if (best <= old_alpha){
                    // Alpha cutoff
                    ttable->get(hash) = {hash, depth, TEntry::UPPERBOUND, best, m, gh->age()};
                }
                else {
                    // Beta cutoff
                    sc->getHH().update(b->getSide() == Piece::White, m, depth);
                    ttable->get(hash) = {hash, depth, TEntry::LOWERBOUND, best, m, gh->age()};
                }                    
                return best;
            }

            if (!keep_searching(params))
                break;
        }

        // Store the best move in the transposition table
        if (!params->shouldStop())
            ttable->get(hash) = {hash, depth, TEntry::EXACT, best, Move(), gh->age()};
        return best;
    }


    /**
     * @brief Search for the best move using the alpha-beta pruning algorithm,
     * thread unsafe only if GameHistory is shared between threads (board is copied)
     * 
     * TODO:
     * - [x] Implement iterative deepening
     * - [ ] Implement quiescence search
     * - [x] Add time management
     * - Enhancements:
     *  - [x] Move ordering
     *  - [ ] SEE
     */
    SearchResult search(Board* board, GameHistory* gh, SearchCache* sc, SearchParams* params)
    {
        if (!board || !gh || !sc)
            return {Move(), 0, ONGOING};

        // Initialize variables
        int eval = MIN;
        Move best_move;
        MoveList ml;
        CacheMoveGen cache;
        Board board_copy(*board);
        int last_irreversible = board_copy.irreversibleIndex();
        void(::gen_legal_moves(&ml, &board_copy, &cache));
        int whotomove = board_copy.getSide() == Piece::White ? 1 : -1;

        // Prepare the timer
        using namespace std::chrono;
        params->start_time = high_resolution_clock::now();
        params->nodes_searched = 0;
        int depth = 1;
        uint64_t time_taken = 1;

        // Iterative deepening
        while(true){
            int alpha = MIN, beta = MAX;
            order_moves(&ml, &board_copy, &cache, sc);

            // Loop through all the moves and evaluate them
            for (size_t i = 0; i < ml.size(); i++){
                Move m(ml[i]);
                ::make(m, &board_copy, gh);
                params->nodes_searched++;
                int score = -negaAlphaBeta(&board_copy, gh, sc, params, alpha, beta, depth);
                ::unmake(m, &board_copy, gh);
                board_copy.irreversibleIndex() = last_irreversible;

                // std::cout << "info currmove " << Piece::notation(ml[i].getFrom(), ml[i].getTo()) 
                //           << " currmovenumber " << i + 1
                //           << " eval " << std::setprecision(2) << float(score * whotomove) / 100.0f << '\n';

                if (score > eval){
                    eval = score;
                    best_move = ml[i];

                    time_taken = duration_cast<milliseconds>(high_resolution_clock::now() - params->start_time).count();
                    std::cout 
                        << "info bestmove " 
                        << Piece::notation(ml[i].getFrom(), ml[i].getTo()) <<": " << float(score * whotomove) / 100.0f 
                        << " currmove " << i + 1 << " (depth=" << depth << " time=" << time_taken << "ms)\n";
                }

                if (!keep_searching(params))
                    break;

                alpha = std::max(alpha, eval);
                if (alpha >= beta)
                    break;
            }
            depth++;

            if (params->depth != -1 && depth > params->depth){
                params->stopSearch();
            }
            if (!keep_searching(params))
                break;
        }

        depth--;
        time_taken = duration_cast<milliseconds>(high_resolution_clock::now() - params->start_time).count();
        time_taken = std::max(time_taken, 1UL);
        std::cout
            << "info bestmove " << Piece::notation(best_move.getFrom(), best_move.getTo())
            << ": " << std::setprecision(2) << float(eval * whotomove) / 100.0f
            << " depth=" << depth 
            << " time=" << time_taken << "ms"
            << " nodes=" << params->nodes_searched
            << " nps=" << (int)(params->nodes_searched / (time_taken / 1000.0)) << '\n';
        
        return SearchResult{best_move, eval, depth, time_taken, ONGOING};
    }
}