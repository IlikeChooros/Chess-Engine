#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <variant>

#include "engine.h"
#include "threads.h"

#include <ui/ui.hpp>

// Universal Chess Interface
namespace uci
{
    // UCI Options
    // Refrence: https://backscattering.de/chess/uci/
    class UCIOptions
    {
    public:
        class Option
        {
        public:

            // Type of the option
            enum Type
            {
                Check,
                Spin,
                Combo,
                Button,
                String,
                Unknown
            };

            // Spin range
            typedef struct 
            {
                int value;
                int def;
                int min;
                int max;
            } SpinRange;

            typedef std::string* StringPointer;
            typedef std::variant<SpinRange, bool, StringPointer> Value;
            typedef std::function<void(chess::Engine&)> Callback;

            // Fields
            Type type{Unknown};
            Value value{};
            Callback callback{nullptr};

            // Constructors
            Option() = default;

            Option(Type type, Value value)
                : type(type), value(value) {}
            
            // Set option as spin
            Option(int default_value, int min, int max)
                : type(Spin)
            {
                this->value = SpinRange{default_value, default_value, min, max};
            }

            // Set option as check
            explicit Option(bool value)
                : type(Check), value(value) {}

            // Create button option with given callback
            explicit Option(Callback callback) 
            : type(Button), value(), callback(callback) {}

            // Set option as string
            Option(std::string value)
                : type(String)
            {
                this->value = new std::string(value);
            }

            // Destructor
            ~Option()
            {
                if (type == String)
                    delete std::get<StringPointer>(value);
            }

            std::string& string() const { return *std::get<StringPointer>(value); }
            bool& boolean() { return std::get<bool>(value); }
            SpinRange& spin() { return std::get<SpinRange>(value); }

            // Copy constructor
            Option& operator=(const Option& other)
            {
                callback    = other.callback;
                type        = other.type;
                if (type == String)
                    value = new std::string(other.string());
                else
                    value = other.value;
                return *this;
            }

            // Assigne value, based on the option type
            Option& operator=(std::string value)
            {
                if (type == String)
                    string() = value;

                else if (type == Check)
                    this->value = value == "true";
                
                else if (type == Spin)
                {
                    SpinRange& spinv = spin();
                    spinv.value = spinv.def;
                    std::sscanf(value.c_str(), "%d", &spinv.value);
                }
                return *this;
            }

            // Convert to string
            std::string toString()
            {
                std::string str = "type ";
                switch (type)
                {
                    case Check:
                        str += "check default ";
                        str += (boolean() ? "true" : "false");
                        break;
                    case Spin: {
                        SpinRange& spin = this->spin();
                        str += "spin default ";
                        str += std::to_string(spin.value);
                        str += " min ";
                        str += std::to_string(spin.min);
                        str += " max ";
                        str += std::to_string(spin.max);
                        }
                        break;
                    case Combo:
                        str += "combo";
                        break;
                    case Button:
                        str += "button";
                        break;
                    case String:
                        str += "string default ";
                        if (string().empty())
                            str += "<empty>";
                        else
                            str += string();
                        break;
                    case Unknown:
                        str += "unknown";
                        break;
                }
                return str;
            }
        };

        std::map<std::string, Option> options;

        // Constructor
        UCIOptions()
        {
            options["Log File"]        = Option(std::string(Log::LOG_FILE));
            options["Hash"]            = Option(chess::SearchCache::DEFAULT_HASH_SIZE, 1, 128);
            options["UCI_AnalyseMode"] = Option(false);
            options["Threads"]         = Option(1, 1, 1);
            options["MultiPV"]         = Option(1, 1, 1);

            options["Clear Hash"]      = Option(
                Option::Callback(
                    [](chess::Engine& e){ e.reset(); }
                )
            );
        }

        // Get option
        Option& operator[](const std::string& key)
        {
            return options[key];
        }

        // Convert to string
        std::string toString()
        {
            std::string str = "";
            for (auto& [key, value] : options)
            {
                str += "option name ";
                str += key;
                str += " ";
                str += value.toString();
                str += "\n";
            }
            return str;
        }

        // Parse UCI options
        void apply(chess::Engine& engine)
        {
            engine.setHashSize(options["Hash"].spin().value);
            engine.setLogFile(options["Log File"].string());
        }

        // Set option
        void set(std::string key, std::string value)
        {
            if (options.find(key) != options.end())
                options[key] = value;
        }

        // Envoke the button callback
        void call(std::string key, chess::Engine& e)
        {
            if (options.find(key) != options.end() 
                && options[key].type == Option::Button)
                options[key].callback(e);
        }
    };

    class UCI
    {
    public:
        UCI();
        ~UCI();
        UCI& operator=(UCI&&);

        /**
         * @brief Get the number of commands left in the queue
         */
        int commandsLeft() { return m_queue.tasksLeft(); }

        void sendCommand(std::string comm);
        void loop(int argc = 0, char** argv = nullptr);
        
    private:
        void setoption(std::istringstream& iss);
        void position(std::istringstream& iss);
        void go(std::istringstream& iss);
        bool parseSide(std::istringstream& iss);
        chess::SearchOptions parseGoOptions(
            std::istringstream& iss, 
            bool allow_infinite = true, 
            bool allow_perft = true
        );
        std::string processCommand(std::string comm);

        UCIOptions    m_options;
        TaskQueue     m_queue;
        chess::Engine m_engine;
        std::mutex    m_mutex;
        std::string   m_command;
        bool          m_ready;
    };
}

