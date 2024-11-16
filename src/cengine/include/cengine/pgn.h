#pragma once

#include <chrono>
#include <string>
#include <map>
#include <iostream>

#include "eval.h"
#include "board.h"

// TODO: remove mangager from the pgn, use only the board

struct PGNFields
{
    std::string Event = "";
    std::string Site = "";
    std::chrono::system_clock::time_point Date = std::chrono::system_clock::now();
    int Round = 1;
    std::string White = "";
    std::string Black = "";
    struct Result{
        chess::GameStatus status = chess::GameStatus::ONGOING;
        int colorWin = chess::Piece::White;
    } Result;
    std::string FEN = "";
};

class PGN
{
public:

    // PGN Field
    class Field
    {
    public:
        std::string name;
        std::string value;

        Field() = default;

        Field(std::string name, std::string value)
            : name(name), value(value) {}
        
        // Assign the value of the field
        Field& operator=(const std::string& value)
        {
            this->value = value;
            return *this;
        }

        // Set integer value
        Field& operator=(int value)
        {
            this->value = std::to_string(value);
            return *this;
        }

        // Set the date value
        Field& operator=(std::chrono::system_clock::time_point value)
        {
            std::time_t t = std::chrono::system_clock::to_time_t(value);
            char date[80];
            std::strftime(date, sizeof(date), "%Y.%m.%d", std::localtime(&t));
            this->value = date;
            return *this;
        }

        // Clear the field
        void clear()
        {
            value.clear();
        }

        // Compare the field with a string
        bool operator==(std::string value)
        {
            return this->value == value;
        }

        // Compare the field with a string
        bool operator!=(std::string value)
        {
            return this->value != value;
        }

        // Convert the field to string using std::string() cast
        operator std::string() const
        {
            // If the value is empty, return an empty string
            if (value.empty())
                return std::string();
            return "[" + name + " \"" + value + "\"]\n";
        }

        // Print to std::cout
        friend std::ostream& operator<<(std::ostream& os, const Field& field)
        {
            os << std::string(field);
            return os;
        }
    };

    // Static methods
    static std::string pgn_game_status(chess::GameStatus status, bool white = true);
    static std::string get_move_notation(chess::Board& m, chess::Move move);

    // Fields
    std::map<std::string, Field> fields;
    static const char* FIELDS_ORDER[9];

    // Constructor
    PGN()
    {
        fields["Event"]  = Field("Event", "");
        fields["Site"]   = Field("Site", "");
        fields["Date"]   = Field("Date", "");
        fields["Round"]  = Field("Round", "1");
        fields["White"]  = Field("White", "");
        fields["Black"]  = Field("Black", "");
        fields["Result"] = Field("Result", "");
        fields["FEN"]    = Field("FEN", "");
        fields["SetUp"]  = Field("SetUp", "");
    }

    // Get the field by name
    Field& operator[](const std::string& field)
    {
        return fields[field];
    }

    void generate_fields(chess::Board);

    std::string pgn(chess::Board board);

    // Convert the PGN to string, only the fields
    operator std::string() const
    {
        std::string pgn = "";
        for (size_t i = 0; i < sizeof(FIELDS_ORDER) / sizeof(FIELDS_ORDER[0]); i++)
        {
            pgn += fields.at(FIELDS_ORDER[i]);
        }
        return pgn;
    }

    // Print the PGN to std::cout
    friend std::ostream& operator<<(std::ostream& os, const PGN& pgn)
    {
        os << std::string(pgn);
        return os;
    }
};