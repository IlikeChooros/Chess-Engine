#pragma once

#include <iostream>
#include <bitset>
#include <string>
#include <chrono>
#include <vector>
#include <queue>
#include <functional>
#include <map>
#include <list>
#include <variant>
#include <emmintrin.h>

#include "types.h"
#include "settings.h"

namespace chess
{

// Argument parser class
// This class is used to parse command line arguments for the chess engine
class ArgParser
{
public:

    typedef std::variant<std::string, int, bool>            arg_value_t;
    typedef std::function<bool(std::string)>                validator_t;
    typedef std::function<void(arg_value_t&, std::string)>  action_t;
    typedef struct {
        std::string name; 
        std::string value;
        validator_t validator; 
        std::string description{};
    } arg_t;

    // Argument types
    enum arg_type_t
    {
        STRING,
        INT,
        BOOL,
        ANY
    };

    // Properties of the argument
    // - required: if the argument is required
    // - help_message: help message for the argument
    // - default_value: default value for the argument
    // - type: type of the argument (e.g., "string", "int", "bool")
    // - value: value of the argument
    // - validator: function to validate the argument (if not set, then matches by type)
    // - action: function to execute when the argument is found
    typedef struct {
        bool required{false}; // if the argument is required
        std::string help_message{}; // help message
        std::string default_value{""}; // default value
        arg_type_t type{arg_type_t::STRING}; // type of the argument (e.g., "string", "int", "bool")
        validator_t validator{nullptr}; // function to validate the argument
        action_t action{nullptr}; // function to execute when the argument is found
    } arg_props_t;


    // Argument properties with value
    typedef struct arg_value_props_t : public arg_props_t {
        arg_value_t value; // value of the argument
        bool set{false}; // if the argument is set

        // Constructor from the arg props
        arg_value_props_t(const arg_props_t& props)
            : arg_props_t(props), value(""), set(false) {}
        
    } arg_value_props_t;

    typedef struct {
        std::vector<std::string> flags; // possible flags
        arg_value_props_t props; // properties of the argument
    } new_arg_t;
    typedef std::list<new_arg_t>                            arg_list_t;
    typedef typename std::list<new_arg_t>::iterator         arg_it_t;
    typedef typename std::list<new_arg_t>::const_iterator   arg_const_it_t;


    // ParsedArguments class
    class ParsedArguments
    {
    public:
        ParsedArguments() = default;
        ParsedArguments(const arg_list_t& args) 
            : m_args(args) {}
        ParsedArguments(const ParsedArguments& other) 
            : m_args(other.m_args) {}

        ParsedArguments& operator=(const ParsedArguments& other) 
        {
            if (this != &other)
                m_args = other.m_args;
            return *this;
        }

        // Checks if the required arguments are set
        void process(const arg_list_t& args);

        // Checks if the argument is set
        bool exists(const std::string& name) const {
            return M_find(name) != m_args.end();
        }

        /**
         * @brief Get the value of the argument
         * @param name The name of the argument
         * @tparam T The type to cast to (e.g., std::string, int, bool)
         * @throws std::runtime_error if the argument is not found or the type does not match
         * @return The value of the argument
         */
        template <typename T>
        const T& get(const std::string& name) const 
        {
            auto it = M_find(name);

            // Check if it exists
            if (it == m_args.end())
                throw std::runtime_error("Argument not found: " + name);
            
            // Check if the type matches
            if (!std::holds_alternative<T>(it->props.value))
                throw std::runtime_error(name + " type mismatch: " 
                    + std::string(typeid(T).name()) + " != "
                    + std::string(M_get_variant_type_name(it->props.value)));

            return std::get<T>(it->props.value);
        }

    private:

        // Get c-style type name of the value
        static const char* M_get_variant_type_name(const arg_value_t& value)
        {
            if (std::holds_alternative<std::string>(value))
                return "string";
            else if (std::holds_alternative<int>(value))
                return "int";
            else if (std::holds_alternative<bool>(value))
                return "bool";
            else
                return "unknown";
        }

        // Find the argument by name, but const
        arg_const_it_t M_find(const std::string& name) const
        {
            return std::find_if(m_args.begin(), m_args.end(), 
                [&name](const new_arg_t& prop){
                    // Check if the name is in the flags
                    return std::find(prop.flags.begin(), prop.flags.end(), name) != prop.flags.end();
            });
        }
    
        // Check if the argument is set
        arg_it_t M_find(const std::string& name)
        {
            return std::find_if(m_args.begin(), m_args.end(), 
                [&name](const new_arg_t& prop){
                    // Check if the name is in the flags
                    return std::find(prop.flags.begin(), prop.flags.end(), name) != prop.flags.end();
            });
        }

        arg_list_t m_args; // list of arguments
    };

    typedef ParsedArguments             arg_map_t;

    // -------- Validators --------

    static bool defaultValidator(std::string value) { return true; }
    static bool booleanValidator(std::string value) { 
        return value.empty() || value == "true" || value == "false" ||
                value == "1" || value == "0";
    }
    static bool intValidator(std::string value) 
    { 
        try {
            std::stoi(value);
            return true;
        } catch (...) {
            return false;
        }
    }
    static bool stringValidator(std::string value) { return !value.empty(); }

    // -------- Actions --------

    static void store(arg_value_t& field, std::string value) 
    {
        // Store the value in the argument, based on the type
        if (std::holds_alternative<std::string>(field))
            std::get<std::string>(field) = value;

        else if (std::holds_alternative<int>(field))
            std::get<int>(field) = std::stoi(value);

        else if (std::holds_alternative<bool>(field))
            std::get<bool>(field) = (value == "true" || value == "1");
    }
    static void storeTrue(arg_value_t& value, std::string) { value = true; }
    static void storeFalse(arg_value_t& value, std::string) { value = false; }
    static void count(arg_value_t& value, std::string) 
    { 
        if (std::holds_alternative<int>(value))
            std::get<int>(value)++;
        else
            value = 1;
    }


    // --------- Constructor ---------

    ArgParser() 
        { M_init(); };

    ArgParser(int argc, char** argv) 
    : m_argc(argc), m_argv(argv) 
        { M_init(); }
    
    /**
     * @brief Set the command line arguments
     * @param argc Number of arguments
     * @param argv Array of arguments
     */
    void setArgs(int argc, char** argv)
    {
        m_argc = argc;
        m_argv = argv;
    }
    
    /**
     * @brief Add an argument to the parser
     * @param name The name of the argument (e.g., "--fen")
     * @param value The default value of the argument
     * @param validator A function to validate the argument value
     * @param description A description of the argument
     */
    void addArg(const std::string& name, const std::string& value = "", 
                std::function<bool(std::string)> validator = defaultValidator, 
                const std::string& description = "")
    {
        
    }

    /**
     * @brief Print the help message for the parser
     */
    void print_help()
    {
        std::cout << "Usage: " << m_argv[0] << " [options]\n";
        std::cout << "Options:\n";
        for (const auto& arg : m_args)
        {
            std::cout << "  "; // print flags
            for (const auto& flag : arg.flags)
                std::cout << flag << " ";
            
            // Print the type of the argument
            std::cout << "\n    type=" << M_type_to_string(arg.props.type);

            // Show the status of the argument
            if (arg.props.required) // required
                std::cout << " (required)";
            else if (!arg.props.default_value.empty()) // default value
                std::cout << " (default: " << arg.props.default_value << ")";
            else
                std::cout << " (optional)";
            
            if (!arg.props.help_message.empty()) // description
                std::cout << "\n    " << arg.props.help_message;

            std::cout << "\n";
        }
    }

    void add_argument(
        const std::vector<std::string>& flags, 
        const arg_props_t& props
    );

    arg_map_t parse();

private:

    /**
     * @brief Initialize the parser with help message
     */
    void M_init()
    {
        add_argument({"--help", "-h"}, {
            .required = false,
            .help_message = "Show this help message",
            .default_value = "false",
            .type = BOOL,
            .validator = booleanValidator,
            .action = [this](arg_value_t&, std::string) { print_help(); }
        });
    }

    /**
     * @brief Convert the argument type to a string representation
     * @param type The argument type
     * @return The string representation of the argument type
     */
    static const char* M_type_to_string(arg_type_t type)
    {
        switch (type)
        {
            case STRING: return "string";
            case INT: return "int";
            case BOOL: return "bool";
            default: return "any";
        }
    }

    int m_argc{0};
    char** m_argv{nullptr};
    arg_list_t m_args;
};

/**
 * @brief Converts a square index to a string representation
 */
inline std::string square_to_str(Square square, bool inverse = true)
{
    if(square < 0 || square > 63)
        return "??";

    std::string str(2, ' ');
    str[0] = 'a' + (square % 8);
    square = inverse ? 7 - square / 8 : square / 8;
    str[1] = '1' + (char)square;
    return str;
}

/**
 * @brief Converts square as string to index format
 * @return -1 if the string is not a valid square
 */
inline Square str_to_square(std::string str, bool inverse = true)
{
    if (str.size() < 2)
        return -1;

    if ((str[0] < 'a' || str[0] > 'h') || (str[1] < '1' || str[1] > '8'))
        return -1;

    int row = str[1] - '1';
    row = inverse ? 7 - row : row;
    return row * 8 + str[0] - 'a';
}

/**
 * @brief Calculate the dot product (sum of b_i * weight_i, where b_i - is the ith bit (0 <= i < 64))
 * @param bb the bitboard (most likely the moves of a piece)
 * @param weights 64 byte array, [0] -> is the top left corner square, [64]-> bottom right corner
 */
constexpr int dot_product(Bitboard bb, Byte weights[])
{
    int product = 0;
    for(int sq = 0, bit = 1; sq < 64; sq++, bit += bit)
    {
        // Could write:
        // if (bb & bit == 0) product += weights[sq]
        // but this is branchless, uses the fact -1 == 111111111...1 (all bits set to 1)
        // and -0 = 0 = 0000000000...0 (all bits set to 0), 
        // so weights[sq] & (-1) = weights[sq], and weights[sq] & (-0) = 0.
        product += weights[sq] & (-((bb & bit) == 0));
    }
    return product;
}

/**
 * @brief Get the LSB1 index (the least significant bit that is 1)
 */
inline int bit_scan_forward(Bitboard bb)
{
    #if defined(__GNUC__) || defined(__clang__)
        return __builtin_ctzll(bb);
    #elif defined(_MSC_VER)
        unsigned long index;
        _BitScanForward64(&index, bb);
        return index;
    #else
        // BitScanForward implementation for other compilers
        // Using de Bruijn multiplication
        // https://www.chessprogramming.org/De_Bruijn_Sequence_Generator
        const uint64_t magic = 0x022fdd63cc95386d; // the 4061955.

        const unsigned int magictable[64] =
        {
            0,  1,  2, 53,  3,  7, 54, 27,
            4, 38, 41,  8, 34, 55, 48, 28,
            62,  5, 39, 46, 44, 42, 22,  9,
            24, 35, 59, 56, 49, 18, 29, 11,
            63, 52,  6, 26, 37, 40, 33, 47,
            61, 45, 43, 21, 23, 58, 17, 10,
            51, 25, 36, 32, 60, 20, 57, 16,
            50, 31, 19, 15, 30, 14, 13, 12,
        };

        return magictable[((bb&-bb)*magic) >> 58];
    #endif
}

/**
 * @brief Pop the least significant bit that is 1 from a bitboard and return its index
 */
inline int pop_lsb1(uint64_t& b)
{
    int lsb1 = b != 0 ? bit_scan_forward(b) : 0;
    b &= b - 1;
    return lsb1;
}

inline int unsafe_pop_lsb1(Bitboard& b)
{
    int lsb = bit_scan_forward(b);
    b &= b - 1;
    return lsb;
}

/**
 * @brief Count the number of bits that are 1 in a bitboard
 */
inline int pop_count(uint64_t b)
{
    #if defined(__GNUC__) || defined(__clang__)
        return __builtin_popcountll(b);
    #elif defined(_MSC_VER)
        return __popcnt64(b);
    #else
        // Popcount implementation for other compilers
        // Using Brian Kernighan's Algorithm
        int count = 0;
        while (b) {
            b &= b - 1;
            count++;
        }
        return count;
    #endif
}

/**
 * @brief Prints a bitboard to the standard output
 */
inline void dbitboard(uint64_t bitboard64)
{
    std::bitset<64> bitboard(bitboard64);
    for(int i = 0; i < 64; i++){
        printf("%*d", 2, int(bitboard[i]));
        if(i % 8 == 7)
            printf(" |%c \n", '8' - i / 8);
    }
    for(int i = 0; i < 8; i++)
        printf("%*c", 2, 'a' + i);
    printf("\n\n");
}


} // namespace chess