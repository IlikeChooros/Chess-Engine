#pragma once

#include <stdio.h>
#include <stdarg.h>

#include "engine.h"
#include "threads.h"

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

            // Value of the option
            typedef union 
            {
                SpinRange spin;
                bool boolean;
                std::string *string;
            } Value;

            // Fields
            Type type;
            Value value;

            // Constructors
            Option() = default;

            Option(Type type, Value value)
                : type(type), value(value) {};
            
            // Set option as spin
            Option(int default_value, int min, int max)
                : type(Spin)
            {
                this->value.spin.value = default_value;
                this->value.spin.def = default_value;
                this->value.spin.min = min;
                this->value.spin.max = max;
            };

            // Set option as check
            Option(bool value)
                : type(Check)
            {
                this->value.boolean = value;
            };

            // Set option as string
            Option(std::string value)
                : type(String)
            {
                this->value.string = new std::string(value);
            };

            // Destructor
            ~Option()
            {
                if (type == String)
                    delete value.string;
            };

            // Copy constructor
            Option& operator=(const Option& other)
            {
                type = other.type;
                if (type == String)
                    value.string = new std::string(*other.value.string);
                else
                    value = other.value;
                return *this;
            };

            // Assigne value, based on the option type
            Option& operator=(std::string value)
            {
                if (type == String)
                    *this->value.string = value;

                else if (type == Check)
                    this->value.boolean = value == "true";
                
                else if (type == Spin)
                {
                    this->value.spin.value = this->value.spin.def;
                    std::sscanf(value.c_str(), "%d", &this->value.spin.value);
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
                        str += (value.boolean ? "true" : "false");
                        break;
                    case Spin:
                        str += "spin default ";
                        str += std::to_string(value.spin.value);
                        str += " min ";
                        str += std::to_string(value.spin.min);
                        str += " max ";
                        str += std::to_string(value.spin.max);
                        break;
                    case Combo:
                        str += "combo";
                        break;
                    case Button:
                        str += "button";
                        break;
                    case String:
                        str += "string default ";
                        if (value.string->empty())
                            str += "<empty>";
                        else
                            str += *value.string;
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
            engine.setHash(options["Hash"].value.spin.value);
            engine.setLogFile(*options["Log File"].value.string);
        }

        // Set option
        void set(std::string key, std::string value)
        {
            if (options.find(key) != options.end())
                options[key] = value;
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
        void loop();
        
    private:
        void setoption(std::istringstream& iss);
        void position(std::istringstream& iss);
        void go(std::istringstream& iss);
        std::string processCommand(std::string comm);

        UCIOptions    m_options;
        TaskQueue     m_queue;
        chess::Engine m_engine;
        std::mutex    m_mutex;
        std::string   m_command;
        bool          m_ready;
    };
}

