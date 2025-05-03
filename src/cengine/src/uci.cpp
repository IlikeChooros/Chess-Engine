#include <cengine/uci.h>

// Universal Chess Interface implementation for the CEngine
// Main refrence:
// https://backscattering.de/chess/uci/

namespace uci
{
    // Help map
    std::map<std::string, const char*> help_map = {
        {"perft", 
            "perft <depth> - Run perft test at given depth, board should be already initialized\n\n"
        },
        {"position", 
            "position [startpos|fen <fen> [moves <move1> ... <moveN>]]\n"
            " - startpos: Start from the initial position\n"
            " - fen <fen>: Start from the given FEN\n"
            " - moves <move1> ... <moveN>: Play the given moves, with format <from><to><promotion>\n"
            "Where <from> and <to> are the square names, and <promotion> is the promotion piece:\n"
            " - q: Queen\n"
            " - r: Rook\n"
            " - b: Bishop\n"
            " - n: Knight\n\n"
            "Example: position startpos\n"
            "Another one: position fen rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8 moves d7c8q\n\n"
        },
        {"go", 
            "go [depth <depth> | nodes <nodes> | movetime <time> | wtime <time> | btime <time> | winc <time> | binc <time> | ponder | infinite]\n"
            " - depth <depth>: Search to the given depth\n"
            "\tExample: go depth 5 (this run search till depth 5 is fully searched)\n"
            " - nodes <nodes>: Search the given number of nodes (not supported yet)\n"
            " - movetime <time>: Search for the given time in milliseconds\n"
            "\tExample: go movetime 1000 (run search for 1 second, depth is unlimited, same as 'go movetime 1000 infinite')\n"
            " - wtime <time>: White time in milliseconds (not supprted yet)\n"
            " - btime <time>: Black time in milliseconds (not supprted yet)\n"
            " - winc <time>: White increment in milliseconds (not supprted yet)\n"
            " - binc <time>: Black increment in milliseconds (not supprted yet)\n"
            " - ponder: Ponder the best move (not supported yet)\n"
            " - infinite: Search indefinitely\n"
            "\t Example: go infinite (run search indefinitely, until 'stop' command is given)\n\n"
        },
        {"uci", "uci - Print the UCI info\n\n"},
        {"setoption", 
            "setoption name <id> [value <x>]\n"
            " - Set value of 'id' option to 'x'\n"
            " - Example: setoption name Hash value 128\n"
            "See 'uci' command for a list of available options\n\n"
        },
        {"ucinewgame", "ucinewgame - Start a new game, resets the search cache\n\n"},
        {"debug", "debug - Toggle debug mode\n\n"},
        {"isready", "isready - Check if the engine is ready\n\n"},
        {"stop", "stop - Stop the search\n\n"},
        {"getfen", "getfen - Get the current FEN (unofficial)\n\n"},
        // {"playgame",
        //     "playgame <position> <your-side> <constraints> (unofficial)\n"
        //     " - Play a game against the engine with simple console ui (see 'makemove' how to make moves')\n"
        //     " - <position>: The position to play from, same as 'position' command\n"
        //     " - <your-side>: The side to play as, by default it's current side, (w | white, b | black)\n"
        //     " - <constraints>: The constraints for the engine, same as 'go' command, \n"
        //     " by default set as 'movetime 3000' (infinite is illegal)\n"
        //     "\tExample: 'playgame startpos w', play a game from the start position as white\n"
        //     "Another one: 'playgame rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8 moves d7c8q b depth 5'\n"
        // },
        {
            "makemove",
                "makemove <move>\n"
                " - Make a move in the current position (used in playgame command)\n"
                " - <move>: The move to make, in the format <from><to><promotion>\n"
                "\tExample: makemove d7c8q (in given position, move the pawn from d7 to c8 and promote to queen)\n"
                "\tAnother one: makemove e2e4 (move the piece from e2 to e4)\n"
        },
        {"help", 
            "\nAvaible commands:\n"
            "uci\n"
            "ucinewgame\n"
            "isready\n"
            "setoption name <id> [value <x>]\n"
            "debug\n"
            "position [startpos|fen <fen> [moves <move1> ... <moveN>]]\n"
            "makemove <move>\n"
            "playgame <position> <your-side> <constraints>\n"
            "go [depth <depth> | nodes <nodes> | movetime <time> | wtime <time> | btime <time> | winc <time> | binc <time> | ponder | infinite]\n"
            "perft <depth>\n"
            "stop\n"
            "getfen\n"
            "help\n"
            "quit\n\n"
            "Try 'help <command>' for more info about a command\n"
            "For example 'help go' will give you more info about the 'go' command\n\n"        
        },
    };


    enum Commands {
        Uci,
        UciNewGame,
        IsReady,
        Position,
        Go,
        Stop,
        GetFen,
        MakeMove,
        Help,
        Quit,
        Debug,
        SetOption,
    };

    std::map<std::string, Commands> command_map = {
        {"uci", Uci},
        {"setoption", SetOption},
        {"ucinewgame", UciNewGame},
        {"debug", Debug},
        {"isready", IsReady},
        {"position", Position},
        {"go", Go},
        {"stop", Stop},
        {"getfen", GetFen},
        {"makemove", MakeMove},
        {"help", Help},
        {"quit", Quit},
    };


    /**
     * @brief Print an error message and throw an exception
     * 
     * @param message The message to print
     */
    void fail(std::string message, ...)
    {
        char buffer[4096];
        va_list args;
        va_start(args, message);
        vsnprintf(buffer, sizeof(buffer), message.c_str(), args);
        va_end(args);
        throw std::runtime_error(buffer);
    }

    int readInt(std::istringstream& iss, std::string message)
    {
        int n;
        if (!(iss >> n)){
            fail(message);
        }
        return n;
    }

    /**
     * @brief Parse the setoption command
     */
    void UCI::setoption(std::istringstream& iss)
    {
        std::string command;
        std::string name;
        std::string value;
        // Read the command and the field name
        iss >> command >> name;

        if (command != "name")
            fail("(setoption): Invalid command: %s\n", command.c_str());

        // Name might have spaces, read until 'value' is found
        while (iss >> command && command != "value")
            name += " " + command;

        // No value just name provided, assuming that's a button
        if (iss.eof() && command != "value")
        {
            m_options.call(name, m_engine);
            return;
        }

        // That's an invalid command, abort
        if (command != "value")
            fail("(setoption): 'value' not found\n");
        
        // Read the value
        iss >> value;
        if (value == "<empty>")
            value = ""; // Empty string

        // Apply the option
        m_options.set(name, value);
        m_options.apply(m_engine);
    }

    /**
     * @brief Parse the position command [startpos|fen <fen> [moves <move1> ... <moveN>]]
     */
    void UCI::position(std::istringstream& iss)
    {
        m_engine.setPosition(iss);
    }

    /**
     * Parse the go command, start the search or run perft 
     * 
     * Options:
     * - depth <depth>: Search to the given depth
     * - nodes <nodes>: Search the given number of nodes
     * - movetime <time>: Search for the given time in milliseconds
     * - wtime <time>: White time in milliseconds (not supprted yet)
     * - btime <time>: Black time in milliseconds (not supprted yet)
     * - winc <time>: White increment in milliseconds (not supprted yet)
     * - binc <time>: Black increment in milliseconds (not supprted yet)
     * - ponder: Ponder the best move (not supported yet)
     * - infinite: Search indefinitely
    */
    chess::SearchOptions UCI::parseGoOptions(std::istringstream& iss, bool allow_infinite, bool allow_perft)
    {
        chess::SearchOptions options;
        std::string command;

        while (iss >> command)
        {
            if (command == "perft" && allow_perft)
            {
                options.setOption("perft", readInt(iss, "(go): Invalid value for perft: " + command));
                return options;;
            }

            if (command == "infinite" && allow_infinite)
            {
                options["infinite"] = true;
                continue;
            }

            if (options.is_boolean(command) && command != "infinite")
            {
                options[command] = true;
                continue;
            }

            options[command] = readInt(iss, "(go): Invalid value for option: " + command);
        }
        return options;
    }



    /**
     * @brief Parse the go command and start the search/perft
     */
    void UCI::go(std::istringstream& iss)
    {
        // Start the search
        auto opts = parseGoOptions(iss, true, true);
        if (opts.is_perft())
        {
            m_engine.perft(opts.perft(), true);
            return;
        }

        m_engine.go(opts);
    }

    /**
     * @brief Process the given command
     * 
     * @param input The command to process
     * @return The output of the command
     */
    std::string UCI::processCommand(std::string input)
    {
        std::istringstream iss(input);
        std::string command, output;
        iss >> command;

        if (command_map.find(command) == command_map.end())
            // return output;
            return "Unknown command: '" + command + "', type 'help' for a list of commands\n";

        Commands comm = command_map[command];
        
        switch (comm){
            case Uci:
                output = "id name CEngine\n";
                output += "id author IlikeChooros\n";
                output += m_options.toString();
                output += "uciok\n";
                break;

            case UciNewGame:
                m_engine.reset();
                break;

            case SetOption:
                setoption(iss);
                break;

            case IsReady:
                output = "readyok\n";
                break;

            case Position:
                position(iss);
                break;

            case Stop:
                m_engine.stop();
                break;

            case Debug:
                // TODO: Implement debug
                // Toggle debug mode
                break;

            case Go:
                go(iss);
                break;

            case GetFen:
                output = m_engine.board().fen() + "\n";
                break;
            
            case MakeMove: {
                std::string move;
                if (iss >> move)
                {
                    if (m_engine.board().isLegal(move))
                        m_engine.board().makeMove(move);
                    // else
                        // output = "Invalid move: " + move + "\n";
                }
            }
                break;
            case Help: {
                // Check if there is a command after help
                std::string help_command;
                if (iss >> help_command)
                {
                    if (help_map.find(help_command) != help_map.end())
                        output = help_map[help_command];
                    // else 
                        // output = "Unknown command: '" + help_command + "', type 'help' for a list of commands\n";
                } 
                else {
                    // Just a help command
                    output = help_map["help"];
                }
            }
                break;

            case Quit:
            default:
                break;
        }
        return output;
    }

    // UCI

    UCI::UCI(): m_queue(1), m_ready(true)
    {
        chess::Engine::base_init();
        m_engine = chess::Engine();

        // Make the output and input have separate buffers
        std::ios_base::sync_with_stdio(false);
        std::cout.setf(std::ios::unitbuf);
    }

    UCI& UCI::operator=(UCI&& other)
    {
        m_engine = std::move(other.m_engine);
        return *this;
    }

    UCI::~UCI()
    {
        m_queue.stop();
    }

    /**
     * @brief Run the UCI loop, read commands from the standard input
     */
    void UCI::loop(int argc, char** argv)
    {
        std::cout << "CEngine UCI ver " << global_settings.version << "\n";

        // Process the command line arguments
        if (argc > 1)
        {
            std::string command;
            for (int i = 1; i < argc; i++)
                command += std::string(argv[i]) + " ";
            
            sendCommand(command);
        }

        // Read from standard input commands
        while(1)
        {
            std::string command;
            std::getline(std::cin, command);

            // if the command is exit, then break
            if(command == "quit" || command == "exit" || command == "q")
            {
                sendCommand("stop");
                break;
            }

            sendCommand(command);
        }
    }

    /**
     * @brief Push new command to the task queue
     */
    void UCI::sendCommand(std::string comm)
    {
        // Send the command to the queue
        m_queue.enqueue([this, comm](){
            // Run the command in the engine, and catch any exceptions
            std::string output;
            try{
                output = processCommand(comm);
            }
            catch(std::exception& e){
                output = e.what();
                // output.clear();
            }
            
            // Print the output
            std::cout << output;
        });
    }
}