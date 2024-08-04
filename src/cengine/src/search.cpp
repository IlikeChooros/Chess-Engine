#include <cengine/search.h>

namespace chess
{
    constexpr int MIN = -(2 << 29),
                  MAX = 2 << 29;

    static std::map<uint64_t, TranspositionEntry> transp_table;


    // Quiescence (no quiet) search, runs aplha beta on captures only and evaluates the position
    int quiescence(Board* board, int alpha, int beta){
        int eval = evaluate(board);
        return eval;
        // if (eval >= beta)
        //     return beta;
        // if (alpha < eval)
        //     alpha = eval;
        
        // GameHistory gh;
        // gh.push(board, Move());
        // MoveList ml;
        // ::gen_captures(&ml, board);
        // for (size_t i = 0; i < ml.size(); i++){
        //     Move m(ml[i]);
        //     ::make(m, board, &gh);
        //     int score = -quiescence(-beta, -alpha, board);
        //     ::unmake(m, board, &gh);

        //     if (score >= beta)
        //         return beta;
        //     if (score > alpha)
        //         alpha = score;
        // }
        // return alpha;
    }

    // int quiescence(Board* b, int alpha, int beta, bool is_max){
    //     int eval = evaluate(b);

    //     GameHistory gh;
    //     gh.push(b, Move());
    //     MoveList ml;
    //     ::gen_captures(&ml, b);
        
    //     if(is_max){
    //         if (eval >= beta)
    //             return beta;
    //         if (alpha < eval)
    //             alpha = eval;

    //         for (size_t i = 0; i < ml.size(); i++){
    //             Move m(ml[i]);
    //             ::make(m, b, &gh);
    //             int score = quiescence(b, alpha, beta, false);
    //             ::unmake(m, b, &gh);

    //             if (score >= beta)
    //                 return beta;
    //             if (score > alpha)
    //                 alpha = score;
    //         }
    //         return alpha;
    //     } else {
    //         if (eval <= alpha)
    //             return alpha;
    //         if (eval < beta)
    //             beta = eval;

    //         for (size_t i = 0; i < ml.size(); i++){
    //             Move m(ml[i]);
    //             ::make(m, b, &gh);
    //             int score = quiescence(b, alpha, beta, true);
    //             ::unmake(m, b, &gh);

    //             if (score <= alpha)
    //                 return alpha;
    //             if (score < beta)
    //                 beta = score;
    //         }
    //         return beta;
    //     }
    // }

    // int alphabetaMax(Board* board, GameHistory* gh, int alpha, int beta, int depth);
    // int alphabetaMin(Board* board, GameHistory* gh, int alpha, int beta, int depth);

    // int alphabetaMax(Board* board, GameHistory* gh, int alpha, int beta, int depth){
    //     if (depth == 0) 
    //         return quiescence(alpha, beta, board);

    //     int best = MIN;
    //     MoveList ml;
    //     void(::gen_legal_moves(&ml, board));

    //     for (size_t i = 0; i < ml.size(); i++){
    //         Move m(ml[i]);
    //         ::make(m, board, gh);
    //         int score = alphabetaMin(board, gh, alpha, beta, depth - 1);
    //         ::unmake(m, board, gh);
            
    //         if (score > best){
    //             best = score;
    //             if (score > alpha)
    //                 alpha = score;
    //         }
    //         if (score >= beta) // Cut-off
    //             return score;
    //     }

    //     return best;
    // }

    // int alphabetaMin(Board* board, GameHistory* gh, int alpha, int beta, int depth){
    //     if (depth == 0) 
    //         return -quiescence(alpha, beta, board);

    //     int best = MAX;
    //     MoveList ml;
    //     void(::gen_legal_moves(&ml, board));

    //     for (size_t i = 0; i < ml.size(); i++){
    //         Move m(ml[i]);
    //         ::make(m, board, gh);
    //         int score = alphabetaMax(board, gh, alpha, beta, depth - 1);
    //         ::unmake(m, board, gh);

    //         if (score < best){
    //             best = score;
    //             if (score < beta)
    //                 alpha = score;
    //         }
    //         if (score <= alpha) // Cut-off
    //             return score;
    //     }

    //     return best;
    // }

    // int alphaBeta(Board* b, GameHistory* gh, int alpha, int beta, int depth, bool is_max){
    //     if (depth == 0)
    //         return quiescence(b, alpha, beta);
        
    //     MoveList ml;
    //     void(::gen_legal_moves(&ml, b));
    //     int best;
    //     Move best_move;

    //     // Lookup transposition table and check for possible cutoffs
    //     uint64_t hash = get_hash(b);
    //     if (transp_table.find(hash) != transp_table.end()){
    //         TranspositionEntry entry = transp_table[hash];
    //         if (entry.depth >= depth){
    //             if (entry.nodeType == TranspositionEntry::PV)
    //                 return entry.score;
    //         }
    //     }

    //     if (is_max){
    //         best = MIN;
    //         for (size_t i = 0; i < ml.size(); i++){
    //             Move m(ml[i]);
    //             ::make(m, b, gh);
    //             int score = alphaBeta(b, gh, alpha, beta, depth - 1, false);
    //             ::unmake(m, b, gh);

    //             if (score > best){
    //                 best = score;
    //                 best_move = m;
    //             }

    //             alpha = std::max(alpha, best);
    //             if (best >= beta){
    //                 // transp_table[hash] = {hash, depth, TranspositionEntry::ALL, best, best_move, gh->age()};
    //                 return best; // beta cutoff
    //             }
    //         }
    //     } else {
    //         best = MAX;
    //         for (size_t i = 0; i < ml.size(); i++){
    //             Move m(ml[i]);
    //             ::make(m, b, gh);
    //             int score = alphaBeta(b, gh, alpha, beta, depth - 1, true);
    //             ::unmake(m, b, gh);

    //             if (score < best){
    //                 best = score;
    //                 best_move = m;
    //             }

    //             beta = std::min(beta, best);
    //             if (best <= alpha){
    //                 // transp_table[hash] = {hash, depth, TranspositionEntry::CUT, best, best_move, gh->age()};
    //                 return best; // alpha cutoff
    //             }
    //         }
    //     }

    //     // Store the best move in the transposition table
    //     transp_table[hash] = {hash, depth, TranspositionEntry::PV, best, best_move, gh->age()};
    //     return best;
    // }

    // Negamax with alpha-beta pruning
    int negaAlphaBeta(Board* b, GameHistory* gh, int alpha, int beta, int depth){
        if (depth == 0)
            return quiescence(b, alpha, beta);

        MoveList ml;
        void(::gen_legal_moves(&ml, b));
        int best = MIN;

        // Lookup transposition table and check for possible cutoffs
        int old_alpha = alpha;
        uint64_t hash = get_hash(b);
        if (transp_table.find(hash) != transp_table.end()){
            TranspositionEntry entry = transp_table[hash];
            if (entry.depth >= depth){
                if (entry.nodeType == TranspositionEntry::EXACT)
                    return entry.score;
                if (entry.nodeType == TranspositionEntry::LOWERBOUND)
                    alpha = std::max(alpha, entry.score);
                if (entry.nodeType == TranspositionEntry::UPPERBOUND)
                    beta = std::min(beta, entry.score);
                
                if (alpha >= beta)
                    return entry.score;
            }
        }

        for (size_t i = 0; i < ml.size(); i++){
            Move m(ml[i]);
            ::make(m, b, gh);
            best = std::max(best, -negaAlphaBeta(b, gh, -beta, -alpha, depth - 1));
            ::unmake(m, b, gh);

            alpha = std::max(alpha, best);
            if (alpha >= beta){
                // Cut-offs
                // The score is a lower bound of the true score
                if (best <= old_alpha)
                    transp_table[hash] = {hash, depth, TranspositionEntry::UPPERBOUND, best, m, gh->age()};
                else 
                    transp_table[hash] = {hash, depth, TranspositionEntry::LOWERBOUND, best, m, gh->age()};
                return best;
            }
        }

        // Store the best move in the transposition table
        transp_table[hash] = {hash, depth, TranspositionEntry::EXACT, best, ml[0], gh->age()};
        return best;
    }


    /**
     * @brief Search for the best move using the alpha-beta pruning algorithm
     */
    SearchResult search(Board* board, GameHistory* gh, SearchParams params)
    {
        int eval = MIN;
        Move best_move;

        MoveList ml;
        void(::gen_legal_moves(&ml, board));

        for (size_t i = 0; i < ml.size(); i++){
            Move m = ml[i];
            ::make(m, board, gh);
            int score = -negaAlphaBeta(board, gh, MIN, MAX, params.depth);
            ::unmake(m, board, gh);

            printf("%s: %d\n", Piece::notation(ml[i].getFrom(), ml[i].getTo()).c_str(), score);

            if (score > eval){
                eval = score;
                best_move = ml[i];
            }
        }

        printf("Best: %s: %d\n", Piece::notation(best_move.getFrom(), best_move.getTo()).c_str(), eval);
        
        return {best_move, eval};
    }
}