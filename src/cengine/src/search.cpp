#include <cengine/search.h>

namespace chess
{
    constexpr int MIN = -(1 << 29),
                  MAX = 1 << 29,
                  MATE = -(1 << 28) + 1;


    // Quiescence (no quiet) search, runs aplha beta on captures only and evaluates the position
    int quiescence(Board* b, GameHistory* gh, int alpha, int beta, int depth){
        // return evaluate(b, nullptr, nullptr);

        // if (depth == 0)
        //     return evaluate(b);
        
        // MoveList ml;
        // GameHistory gh;
        // gh.push(b, Move());
        // ::gen_captures(&ml, b);

        // if (ml.size() == 0)
        //     return evaluate(b);
        
        // int best = MIN;
        // for (size_t i = 0; i < ml.size(); i++){
        //     Move m(ml[i]);
        //     ::make(m, b, &gh);
        //     best = std::max(best, -quiescence(b, -beta, -alpha, depth - 1));
        //     ::unmake(m, b, &gh);

        //     alpha = std::max(alpha, best);
        //     if (alpha >= beta){
        //         return best;
        //     }
        // }
        // return best;

        // GameHistory gh;
        // gh.push(b, Move());
        // MoveList ml;
        // void(::gen_captures(&ml, b));
        // int best = MIN;

        // for (size_t i = 0; i < ml.size(); i++){
        //     Move m(ml[i]);
        //     ::make(m, b, &gh);
        //     best = std::max(best, -quiescence(b, -beta, -alpha));
        //     ::unmake(m, b, &gh);

        //     alpha = std::max(alpha, best);
        //     if (alpha >= beta){
        //         return best;
        //     }
        // }

        // return best;
        int eval = evaluate(b, nullptr, nullptr);
        if (eval >= beta)
            return beta;
        alpha = std::max(alpha, eval);
        
        MoveList ml;
        ::gen_captures(&ml, b);

        for (size_t i = 0; i < ml.size(); i++){
            Move m(ml[i]);
            ::make(m, b, gh);
            int score = -quiescence(b, gh, -beta, -alpha, 0);
            ::unmake(m, b, gh);

            if (score >= beta)
                return beta;
            if (score > alpha)
                alpha = score;
        }
        return alpha;
    }

    // Negamax with alpha-beta pruning
    int negaAlphaBeta(Board* b, GameHistory* gh, SearchCache* sc, int alpha, int beta, int depth){
        if (depth == 0)
            return quiescence(b, gh, alpha, beta, 8);

        CacheMoveGen cache;
        MoveList ml;
        void(::gen_legal_moves(&ml, b, &cache));
        order_moves(&ml, b, &cache, sc);
        int best = MATE - depth;
        int last_irreversible = b->irreversibleIndex();
        TTable<TEntry> *ttable = &sc->getTT();

        // Look for draw conditions and check if the game is over
        uint64_t hash = get_hash(b);
        auto status = get_status(b, gh, &ml, &cache);
        if (status != ONGOING){
            if (status == DRAW || status == STALEMATE)
                best = 0;
            ttable->get(hash) = {hash, depth, TEntry::EXACT, best, Move(), gh->age()};
            return best;
        }

        // Lookup transposition table and check for possible cutoffs
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
            best = std::max(best, -negaAlphaBeta(b, gh, sc, -beta, -alpha, depth - 1));
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
        }

        // Store the best move in the transposition table
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
        int side_eval = board_copy.getSide() == Piece::White ? 1 : -1;

        // Prepare the timer
        using namespace std::chrono;
        high_resolution_clock::time_point t1 = high_resolution_clock::now();
        int depth = 1;
        uint64_t time_taken = 0;

        // Iterative deepening
        while(true){
            int alpha = MIN, beta = MAX;
            order_moves(&ml, &board_copy, &cache, sc);

            // Loop through all the moves and evaluate them
            for (size_t i = 0; i < ml.size(); i++){
                Move m(ml[i]);
                ::make(m, &board_copy, gh);
                int score = -negaAlphaBeta(&board_copy, gh, sc, -beta, -alpha, depth);
                ::unmake(m, &board_copy, gh);
                board_copy.irreversibleIndex() = last_irreversible;


                if (score > eval){
                    eval = score;
                    best_move = ml[i];

                    std::cout << Piece::notation(ml[i].getFrom(), ml[i].getTo()) <<
                        ": " << float(score * side_eval) / 100.0f <<
                        " (depth=" << depth << " time=" << time_taken << "ms)\n";
                }

                if (params->shouldStop()){
                    break;
                }
                if (params->infinite){
                    continue;
                }
                // Update the timer, check if time is up
                time_taken = duration_cast<milliseconds>(high_resolution_clock::now() - t1).count();
                if (time_taken >= params->movetime){
                    params->stopSearch();
                    break;
                }
            }
            depth++;

            if (params->depth != -1 && depth > params->depth){
                params->stopSearch();
            }
            // Check if the search should stop
            if (params->shouldStop()){
                break;
            }   
        }

        depth--;
        std::cout<< "bestmove = " << Piece::notation(best_move.getFrom(), best_move.getTo()) <<
            ": " << std::setprecision(2) << float(eval * side_eval) / 100.0f <<
            " (depth=" << depth << " time=" << time_taken << "ms)\n\n";
        
        return SearchResult{best_move, eval, depth, time_taken, ONGOING};
    }
}