#pragma once

#include <string.h>
#include <memory>
#include <list>
#include <stdio.h>
#include <vector>
#include <cmath>
#include <chrono>
#include <thread>
#include <mutex>

#include "board.h"
#include "move.h"
#include "utils.h"
#include "castling_rights.h"
#include "move_gen.h"
#include "history.h"
#include "eval.h"
#include "search.h"
#include "magic_bitboards.h"

namespace chess
{
    // Implements the Manager class, has simple to use API for the user
    class ManagerImpl
    {
    public:
        static SearchCache search_cache;

        ManagerImpl(Board* board = nullptr);
        ManagerImpl(const ManagerImpl& other) = delete;
        ~ManagerImpl();
        ManagerImpl& operator=(ManagerImpl&& other);

        void reload();

        /**
         * @brief Resets the manager, clearing all the history, move list and search cache (unlike reload)
         */
        inline void reset()
        {
            this->search_cache.getHH().clear();
            this->search_cache.getTT().clear();
            reload();
        }

        /**
         * @brief Returns the current game status
         * @return The current game status
         */
        inline GameStatus getStatus() { return get_status(board, &history, &move_list); }

        /**
         * @brief Initializes all engine components
         */
        static inline void init() {
            init_board();
            init_eval();
            init_hashing();
            init_magics(false);
        }

        /**
         * @brief Generates all legal moves for the current board state
         * @return The number of moves generated
         */
        inline int generateMoves() { 
            n_moves = ::gen_legal_moves(&move_list, board, &cache); 
            return n_moves;
        }

        /**
         * @brief Searches for the best move in the current position, 
         * if history is not read/modifed by another thread, then thread safe
         * @return The best move found & its score
         */
        inline SearchResult& search() noexcept {
            // Copy all the data to the search thread
            Board board_copy;
            GameHistory history_copy;
            {
                std::lock_guard<std::mutex> l(mutex);
                board_copy = *board;
                history_copy = history;
                search_params.resetStop();
            }  
            ::chess::search(&board_copy, &history_copy, &search_cache, &search_params, &search_result); 
            return search_result;
        }

        /**
         * @brief Searches for the best move in the current position in async mode
         */
        inline void searchAsync() noexcept {
            if (search_thread.joinable()){
                search_thread.join();
            }
            search_thread = std::thread([this](){
                search();
            });
        }

        /**
         * @brief Check if the search is running
         */
        inline bool searchRunning() noexcept {          
            std::unique_lock<std::mutex> l(search_params.mutex);
            return search_params.is_running;
        }

        /**
         * @brief Stops the search running in another thread if it is running
         */
        inline void stopSearchAsync() noexcept {
            search_params.stopSearch();
            if (search_thread.joinable()){
                search_thread.join();
            }
            using namespace std::chrono_literals;
            while(1){
                {
                    std::unique_lock<std::mutex> l(search_params.mutex);
                    if (!search_params.is_running){
                        break;
                    }
                }
                std::this_thread::sleep_for(10ms);
            }
        }

        /**
         * @brief Get the search result, should be called after finished search
         */
        inline SearchResult& getSearchResult() noexcept { return search_result; }

        /**
         * @brief Pushes the current move to the history stack, thread safe
         */
        inline void pushHistory() noexcept { 
            std::lock_guard<std::mutex> l(mutex);
            history.push(board, curr_move); 
        }

        /**
         * @brief Makes a move, updating the board, the side to move and the move history, thread safe
         * @attention User should call `generateMoves()` after calling this function
         */
        inline void make(Move& move) { 
            std::lock_guard<std::mutex> l(mutex); 
            ::make(move, board, &history); 
            curr_move = move;
        }

        /**
         * @brief Unmakes current move, restoring the board to the previous state, 
         * may be called only once after `make()`, thread safe
         * @attention User should call `generateMoves()` after calling this function
         */
        inline void unmake() noexcept { 
            std::lock_guard<std::mutex> l(mutex); 
            ::unmake(curr_move, board, &history); 
            curr_move = history.back().move;
        }

        /**
         * @brief Sets the search parameters for the search function
         */
        inline void setSearchParams(SearchParams params) { search_params = params; }

        inline std::mutex& getMutex() { return mutex; }

        std::mutex mutex;
        std::thread search_thread;
        SearchResult search_result;
        SearchParams search_params;
        CacheMoveGen cache;
        GameHistory history;
        MoveList move_list;
        int n_moves;
        Board* board;
        Move curr_move;
    };
}