#pragma once

#include <unordered_map>

#include "move.h"
#include "types.h"


// Stores, just the hash.
// Will use always replace policy in table
struct BaseTTEntry
{
    uint64_t hash = 0UL;
};

// Stores age and depth of the entry, then
// when storing the table, will change the 
// storing policy to depth and age based
struct DepthBasedEntry : public BaseTTEntry
{
    int          age   : 16;
    chess::Depth depth : 16;
};

// Transposition table entry (for search)
// hash: Zobrist hash of the position
// depth: Depth of the search
// nodeType: Type of node (exact, lowerbound, upperbound)
// score: Score of the position
// bestMove: Best move found
// age: Age of the entry
struct TEntry : public DepthBasedEntry
{
    static constexpr int EXACT = 0; // Exact score
    static constexpr int LOWERBOUND = 1; // Lower bound (on beta cutoff)
    static constexpr int UPPERBOUND = 2; // Upper bound (on alpha cutoff)

    int nodeType : 3;
    int score    : 29;
    chess::Move bestMove;
};

// Transposition table class, implemented as a fixed size vector
// T: Type of the entry (default is TEntry)
template <typename T = TEntry, std::enable_if_t<std::is_base_of<BaseTTEntry, T>::value, bool> = true>
class TTable
{
public:
    typedef typename std::is_base_of<BaseTTEntry, T> IsBase;
    typedef typename std::is_base_of<DepthBasedEntry, T> isDepthBased; 
    typedef typename std::vector<T> TableType;

    // Function to get the key to table, based on the hash and max_size
    static constexpr int get_key(uint64_t hash, uint64_t max_size) {
        return hash % max_size;
    }

    // Constructor with size in MB of the table
    TTable(size_t sizeMB = 1) noexcept 
    {
        m_max_size = sizeMB * (1 << 20) / sizeof(T);
        m_table.resize(m_max_size);
        clear();
    }

    TTable(const TableType& other) : 
        m_table(other.m_table), m_max_size(other.m_max_size) {}
    
    TTable& operator=(const TableType& other)
    {
        m_table    = other.m_table;
        m_max_size = other.m_max_size;
        return *this;
    }
    
    // Set all entries to default state
    inline void clear() noexcept 
    {
        if constexpr (isDepthBased())
        {
            M_clear_depthbased();
        }
        else
        {
            for (auto& e : m_table)
                e.hash = 0ULL;
        }
    }

    // Store new entry, depending on the type of entry
    // If depth based, it might not be stored
    // Else, will just always replace the entry
    inline void store(T e) noexcept 
    {
        if constexpr (isDepthBased())
        {
            M_store_depthbased(e);
        }
        else
        {
            // Always replace policy
            m_table[get_key(e.hash, m_max_size)] = e;
        }        
    }

    // See if given hash exists
    inline bool contains(uint64_t hash) noexcept 
    {
        return m_table[get_key(hash, m_max_size)].hash == hash;
    }

    // Get the entry
    inline T& get(uint64_t hash)
    {
        return m_table[get_key(hash, m_max_size)];
    }

    // Returns vector of entries 
    inline TableType& getTable() noexcept 
    {
        return m_table;
    }

    // Get the load factor
    inline float load_factor()
    {
        int used = 0;
        for (auto& e : m_table)
            used += e.hash != 0 ? 1 : 0;
            
        return float(used) / m_max_size;
    }

private:

    // Will store the entry, based on age and depth replacment policy
    void M_store_depthbased(T& entry)
    {
        T& prev = get(entry.hash);

        // Check if this entry has higher depth assigned
        if (entry.depth > prev.depth)
        {
            m_table[get_key(entry.hash, m_max_size)] = entry;
        }
        else
        {
            // Is quite old, so override anyway
            if ((prev.age + 4) < entry.age)
            {
                m_table[get_key(entry.hash, m_max_size)] = entry;
            }
        }
    }

    // Clear the table, if entry is depth based
    void M_clear_depthbased()
    {
        for(auto& e : m_table)
        {
            e.hash  = 0UL;
            e.depth = 0;
            e.age   = -1;
        }
    }

    TableType    m_table;
    uint64_t  m_max_size;
};