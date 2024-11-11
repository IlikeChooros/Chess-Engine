#pragma once

#include <unordered_map>

#include "move.h"
#include "types.h"


// Transposition table entry
// hash: Zobrist hash of the position
// depth: Depth of the search
// nodeType: Type of node (exact, lowerbound, upperbound)
// score: Score of the position
// bestMove: Best move found
// age: Age of the entry
struct TEntry
{
    static constexpr int EXACT = 0; // Exact score
    static constexpr int LOWERBOUND = 1; // Lower bound (on beta cutoff)
    static constexpr int UPPERBOUND = 2; // Upper bound (on alpha cutoff)

    uint64_t hash;
    int depth;
    int nodeType;
    int score;
    chess::Move bestMove;
    int age;
};


// Transposition table class, implemented as a hash table
// T: Type of the entry (default is TEntry)
template <typename T = TEntry>
class TTable
{
public:
    typedef typename std::unordered_map<uint64_t, T> TableType;

    TTable(size_t sizeMB = 1) noexcept {
        const size_t size = sizeMB * 1000000 / sizeof(T);
        table.reserve(size);
    }

    inline void clear() noexcept {
        table.clear();
    }

    inline void store(uint64_t hash, T e) noexcept {
        table[hash] = e;
    }

    inline bool contains(uint64_t hash) noexcept {
        return table.find(hash) != table.end();
    }

    inline T& get(uint64_t hash){
        return table[hash];
    }

    inline TableType& getTable() noexcept {
        return table;
    }

private:
    TableType table;
};