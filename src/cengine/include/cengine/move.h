#pragma once

#include <vector>
#include <cstring>

#include "utils.h"
#include "types.h"

// Class representing a move, has a from, to and flags. Internally stored as a 32 bit integer
namespace chess
{

class Move
{
    public:

    typedef uint16_t move_t;

    // Mask for the from and to fields (6 bits)
    static const uint32_t MASK_MOVE = 0b111111;
    static const int bits = 16; // 6 + 6 + 4
    static const uint32_t nullMove = 0;

    // Flag structure (4 bits)
    // first bit -> special bit 1 (0b000x)
    // second bit -> special bit 2 (0b00x0)
    // third bit -> capture (0b0x00)
    // fourth bit -> promotion (0bx000)
    static const uint32_t FLAG_NONE = 0b0000; // No flags
    static const uint32_t FLAG_DOUBLE_PAWN = 0b0001; // Double pawn move
    static const uint32_t FLAG_KING_CASTLE = 0b0010; // King castle 0b0010
    static const uint32_t FLAG_QUEEN_CASTLE = 0b0011; // Queen castle 0b0011

    // Capture flags
    static const uint32_t FLAG_CAPTURE = 0b0100;
    static const uint32_t FLAG_ENPASSANT_CAPTURE = 0b0101;

    // Promotion flags
    static const uint32_t MASK_PROMOTION_PIECE = 0b0011;
    static const uint32_t FLAG_PROMOTION = 0b1000;
    static const uint32_t FLAG_KNIGHT_PROMOTION = 0b1000;
    static const uint32_t FLAG_BISHOP_PROMOTION = 0b1001;
    static const uint32_t FLAG_ROOK_PROMOTION = 0b1010;
    static const uint32_t FLAG_QUEEN_PROMOTION = 0b1011;

    // Promotion capture flags
    static const uint32_t FLAG_KNIGHT_PROMOTION_CAPTURE = 0b1100;
    static const uint32_t FLAG_BISHOP_PROMOTION_CAPTURE = 0b1101;
    static const uint32_t FLAG_ROOK_PROMOTION_CAPTURE = 0b1110;
    static const uint32_t FLAG_QUEEN_PROMOTION_CAPTURE = 0b1111;
    
    // Fast move creation
    static constexpr uint16_t fmove(uint32_t from, uint32_t to, uint32_t flags) {
        return (flags << 12) | (from << 6) | to;
    }

    /**
     * @brief Create a move from a string notation (e.g. e2e4, a7a8q),
     * user should call `setFlags` to set the flags for the move
     */
    static Move fromUci(std::string move) 
    {
        if (move.size() < 4)
            return Move(nullMove);
        
        int from = str_to_square(move.substr(0, 2));
        int to = str_to_square(move.substr(2, 2));

        if (from == -1 || to == -1)
            return Move(nullMove);
        
        if (move.size() == 5)
        {
            int promotion = getPromotionPiece(move[4]);
            if (promotion == -1)
                return Move(nullMove);
            
            return Move(from, to, FLAG_PROMOTION | promotion);
        }

        return Move(from, to, FLAG_NONE);
    }

    Move() : m_move(0) {};
    Move(uint32_t from, uint32_t to, uint32_t flags) :
        m_move(fmove(from, to, flags)) {}; // First 6 bits: to, next 6 bits: from, rest: flags (4 bits)
          
    Move(uint32_t move) : m_move(move) {};
    Move(const Move& other) : m_move(other.m_move) {};

    // Same as `fromUci` 
    Move(std::string move) {
        (void)fromUci(move);
    }

    /**
     * @brief Set the flags for the move use only if 
     * the move was created with the string notation constructor
     */
    bool setFlags(std::string move, std::vector<uint32_t> flags) {
        if (flags.empty()){
            return false;
        }

        uint32_t flag = flags[0];
        if (move.size() == 5){
            int promotion = getPromotionPiece(move[4]);

            if (promotion == -1 || !(flag & FLAG_PROMOTION)){
                return false;
            }
            
            flag &= (FLAG_PROMOTION | FLAG_CAPTURE);
            flag |= promotion;
        }

        m_move |= flag << 12;
        return true;
    }

    uint32_t getFrom() const {return (m_move >> 6) & MASK_MOVE;};
    uint32_t getTo() const {return m_move & MASK_MOVE;};
    uint32_t getFlags() const {return m_move >> 12;};
    
    // Get the move as a 16 bit integer
    uint16_t get() const {return m_move;};

    // Get the move part of the move (from and to)
    uint16_t movePart() const { return m_move & (MASK_MOVE << 6 | MASK_MOVE);};

    bool isCapture() const {return getFlags() & FLAG_CAPTURE;};
    bool isPromotion() const {return getFlags() & FLAG_PROMOTION;};
    bool isPromotionCapture() const {return (getFlags() & (FLAG_CAPTURE | FLAG_PROMOTION)) == (FLAG_CAPTURE | FLAG_PROMOTION);};
    bool isDoubleMove() const {return getFlags() == FLAG_DOUBLE_PAWN;};
    bool isEnPassant() const {return getFlags() == FLAG_ENPASSANT_CAPTURE;};
    bool isQueenCastle() const {return getFlags() == FLAG_QUEEN_CASTLE;};
    bool isKingCastle() const {return getFlags() == FLAG_KING_CASTLE;};
    bool isCastle() const {return isQueenCastle() || isKingCastle();};
    bool isNull() const {return m_move == nullMove;};

    /**
     * @brief Get the promotion piece, if the move is a promotion
     * @return 0 - Knight, 1 - Bishop, 2 - Rook, 3 - Queen
     */
    int getPromotionPiece() const {return getFlags() & MASK_PROMOTION_PIECE;};

    /**
     * @brief Get the promotion character, if the move is a promotion
     * @return n - Knight, b - Bishop, r - Rook, q - Queen, 0 - Invalid
     */
    char getPromotionChar() const {
        switch(getPromotionPiece()){
            case 0:
                return 'n';
            case 1:
                return 'b';
            case 2:
                return 'r';
            case 3:
                return 'q';
            default:
                return 0;
        }
    };

    /**
     * @brief Get the promotion piece, if the move is a promotion
     * @param c The character representing the piece
     * @return 0 - Knight, 1 - Bishop, 2 - Rook, 3 - Queen, -1 - Invalid
     */
    static int getPromotionPiece(char c) {
        switch(c){
            case 'n':
                return 0;
            case 'b':
                return 1;
            case 'r':
                return 2;
            case 'q':
                return 3;
            default:
                return -1;
        }
    };

    // Get the move in UCI notation, (e.g. e2e4) (no promotion info)
    static std::string notation(int from, int to){
        return square_to_str(from) + square_to_str(to);
    }

    /**
     * @brief Get the move in UCI notation, (e.g. e2e4, a7a8q)
     */
    std::string notation() const 
    {
        std::string str = square_to_str(getFrom()) + square_to_str(getTo());
        if (isPromotion())
            str += getPromotionChar();
        
        return str;
    }

    operator int() const {return m_move;};

    Move& operator=(const Move& other) 
    {
        m_move = other.m_move;
        return *this;
    };

    bool operator==(uint32_t other) const 
    {
        return m_move == other;
    };

    bool operator!=(uint32_t other) const 
    {
        return m_move != other;
    };

    bool operator==(const Move& other) const 
    {
        return m_move == other.m_move;
    };

    bool operator!=(const Move& other) const 
    {
        return m_move != other.m_move;
    };
    
    private:
    uint16_t m_move;
};

// Move list class, stores a list of moves
class MoveList
{
    public:
    typedef uint16_t move_t;

    // Iterator for the move list
    class MoveListIterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = move_t;
        using pointer = move_t*;
        using reference = move_t&;

        MoveListIterator(move_t* ptr): m_ptr(ptr) {}

        reference operator*() const {return *m_ptr;}
        pointer operator->() {return m_ptr;}

        MoveListIterator& operator++() {
            m_ptr++;
            return *this;
        }

        MoveListIterator operator++(int) {
            MoveListIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        MoveListIterator& operator--() {
            m_ptr--;
            return *this;
        }

        MoveListIterator operator--(int) {
            MoveListIterator tmp = *this;
            --(*this);
            return tmp;
        }

        ptrdiff_t operator-(const MoveListIterator& other) {
            return m_ptr - other.m_ptr;
        }

        MoveListIterator operator-(int n) {
            return MoveListIterator(m_ptr - n);
        }

        MoveListIterator operator+(ptrdiff_t n) {
            return MoveListIterator(m_ptr + n);
        }

        friend bool operator<(const MoveListIterator& lhs, const MoveListIterator& rhs) {
            return lhs.m_ptr < rhs.m_ptr;
        }

        friend bool operator>(const MoveListIterator& lhs, const MoveListIterator& rhs) {
            return lhs.m_ptr > rhs.m_ptr;
        }

        friend bool operator== (const MoveListIterator& a, const MoveListIterator& b) {
            return a.m_ptr == b.m_ptr;
        }

        friend bool operator!= (const MoveListIterator& a, const MoveListIterator& b) {
            return a.m_ptr != b.m_ptr;
        }
    private:
        pointer m_ptr;
    };

    typedef MoveListIterator iterator;
    typedef std::reverse_iterator<iterator> reverse_iterator;


    MoveList(): n_moves(0) {}
    MoveList(const MoveList& other){
        *this = other;
    }

    MoveList& operator=(const MoveList& other)
    {
        n_moves = other.n_moves;
        memcpy(moves, other.moves, other.n_moves * sizeof(move_t));
        return *this;
    }

    inline size_t size() const {return n_moves;}
    inline static constexpr size_t capacity() {return 256;}
    inline void add(move_t move) {moves[n_moves++] = move;}
    inline void add(const Move& move) {add(move.get());}
    inline void clear() {n_moves = 0;}
    inline Move operator[](size_t i) {return Move(moves[i]);}
    inline iterator begin() {return iterator(moves);}
    inline iterator end() {return iterator(moves + n_moves);}
    inline reverse_iterator rbegin() {return reverse_iterator(end());}
    inline reverse_iterator rend() {return reverse_iterator(begin());}
    inline const move_t* data() const {return moves;}

    move_t moves[256];
    size_t n_moves;
};

} // namespace chess