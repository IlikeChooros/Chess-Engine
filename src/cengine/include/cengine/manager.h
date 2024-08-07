#pragma once

#include "manager_impl.h"

namespace chess
{
    class Manager
    {
    public:
        typedef struct {
            uint32_t x, y;
            uint32_t flags;
        } PieceMoveInfo;

        Manager(Board* board = nullptr);
        Manager(const Manager& other) = delete;
        Manager& operator=(Manager&& other);
        
        bool movePiece(uint32_t from, uint32_t to, int flags = -1);
        std::list<PieceMoveInfo> getPieceMoves(uint32_t from);
        std::vector<uint32_t> getFlags(uint32_t from, uint32_t to);
        bool isPromotion(uint32_t from, uint32_t to);

        /**
         * @brief Initialize the boards, search and evaluation
         */
        inline void init() { m_impl->init(); }

        /**
         * @brief Search for the best move
         */
        inline void search() { m_search_result = m_impl->search(); }

        /**
         * @brief Make the engine move, search should be called before this
         */
        inline void makeEngineMove() { 
            if (!m_search_result.move)
                return;
            m_impl->make(m_search_result.move);
            m_impl->generateMoves();
        }

        /**
         * @brief Get the search result, should be called after makeEngineMove
         */
        inline SearchResult getSearchResult() { return m_search_result; }

        /**
         * @brief Reload the manager, should be called after changing the board (for example after loading a FEN string)
         */
        inline void reload() { m_impl->reload(); }

        /**
         * @brief Unmake current move
         */
        inline void unmake() { m_impl->unmake(); }

        /**
         * @brief Generate moves for the current board state
         */
        inline int generateMoves() { return m_impl->generateMoves(); }

        /**
         * @brief Get the current game state
         */
        inline GameStatus getStatus() { return m_impl->getStatus(); }

        /**
         * @brief Get the board
         */
        inline Board* board() { return m_impl->board; }

        /**
         * @brief Get the implementation
         */
        inline ManagerImpl* impl() { return m_impl.get(); }

    private:
        std::unique_ptr<ManagerImpl> m_impl;
        SearchResult m_search_result;
        bool is_searching;
    };
}