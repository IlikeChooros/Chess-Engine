#include <cengine/uci.h>

// Universal Chess Interface implementation for the CEngine
// Main refrence:
// https://backscattering.de/chess/uci/

namespace uci
{
    // Help map
    std::map<std::string, std::string> help_map = {
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
        {"help", 
            "\nAvaible commands:\n"
            "uci\n"
            "ucinewgame\n"
            "isready\n"
            "position [startpos|fen <fen> [moves <move1> ... <moveN>]]\n"
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
        std::string fen;
        std::string command;
        iss >> command;

        if (command == "startpos")
        {
            // Start from the initial position and possibly parse 'moves'
            fen = chess::Board::START_FEN;
            if(iss >> command && command == "moves")
            {
                fen += " moves ";
                while (iss >> command)
                    fen += command + " ";
            }
        }
        else if (command == "fen")
        {
            int n_section = 0;
            while (iss >> command)
            {
                fen += command + " ";
                n_section++;
            }

            if (n_section < 6)
                fail("(position): Not enough sections in FEN: %s\n", fen.c_str());
        }
        else 
        {
            fail("(position): Invalid command: %s\n", command.c_str());
        }

        m_engine.setPosition(fen);
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
    void UCI::go(std::istringstream& iss)
    {
        chess::SearchOptions options;
        std::string command;

        while (iss >> command)
        {
            if (command == "perft")
            {
                m_engine.perft(readInt(iss, "(go): Perft depth not specified or invalid\n"));
                return;
            }

            if (options.is_boolean(command))
            {
                options[command] = true;
                continue;
            }

            options[command] = readInt(iss, "(go): Invalid value for option: " + command);
        }

        // Start the search
        m_engine.go(options);
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

        if (command_map.find(command) == command_map.end()){
            return "Unknown command: '" + command + "', type 'help' for a list of commands\n";
        }

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
                    else
                        output = "Invalid move: " + move + "\n";
                }
            }
                break;

            case Help: {
                // Check if there is a command after help
                std::string help_command;
                if (iss >> help_command){
                    if (help_map.find(help_command) != help_map.end()){
                        output = help_map[help_command];
                    } else {
                        output = "Unknown command: '" + help_command + "', type 'help' for a list of commands\n";
                    }
                } else {
                    // Just a help command
                    output = help_map["help"];
                }
            }
                break;

            case Quit:
                output = "Bye!\n";
                break;

            default:
                output = "Unknown command: '" + command + "', type 'help' for a list of commands\n";
                break;
        }
        return output;
    }

    // UCI

    UCI::UCI(): m_queue(1), m_ready(true)
    {
        chess::Engine::init();
        m_engine = chess::Engine();

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
                break;

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
            }
            
            // Print the output
            std::cout << output;
        });
    }
}