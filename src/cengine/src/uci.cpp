#include <cengine/uci.h>

// Universal Chess Interface implementation for the CEngine
// Main refrence:
// https://backscattering.de/chess/uci/

namespace uci
{


    std::unordered_map<std::string, std::string> help_map = {
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
            "Example: position startpos moves e2e4 e7e5 g1f3 b8c6 f1c4 g8f6 d2d3\n"
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
        {"ucinewgame", "ucinewgame - Start a new game, resets the search cache\n\n"},
        {"isready", "isready - Check if the engine is ready\n\n"},
        {"stop", "stop - Stop the search\n\n"},
        {"getfen", "getfen - Get the current FEN (unofficial)\n\n"},
        {"help", "\nAvaible commands:\n"
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

    /**
     * @brief Print an error message and throw an exception
     * 
     * @param message The message to print
     */
    void fail(const char* message, ...)
    {
        char buffer[4096];
        va_list args;
        va_start(args, message);
        vsnprintf(buffer, sizeof(buffer), message, args);
        va_end(args);
        throw std::runtime_error(buffer);
    }

    int readInt(std::istringstream& iss, const char* message)
    {
        int n;
        if (!(iss >> n)){
            fail(message);
        }
        return n;
    }

    /**
     * @brief Run perft test at given depth, board should be already initialized,
     * thread safe
     */
    void perft(chess::Manager* manager, int depth)
    {
        using namespace chess;
        Board copy(*manager->impl()->board);
        test::Perft perft(&copy);
        perft.setPrint(true);
        perft.run(depth, copy.getFen());
    }

    /**
     * @brief Parse the position command [startpos|fen <fen> [moves <move1> ... <moveN>]]
     */
    void position(chess::Manager* manager, std::istringstream& iss)
    {
        std::string fen;
        std::string command;
        iss >> command;
        if (command == "startpos"){
            fen = chess::Board::startFen;
            if ((iss >> command) && command != "moves"){
                fail("(position): Invalid command after startpos: %s\n", command.c_str());
            }
            fen += " moves ";
            while (iss >> command){ // parse moves
                fen += command + " ";
            }
        }
        else if (command == "fen"){
            int n_section = 0;
            while (iss >> command){
                fen += command + " ";
                n_section++;
            }
            if (n_section < 6){
                fail("(position): Not enough sections in FEN: %s\n", fen.c_str());
            }
        } else {
            fail("(position): Invalid command: %s\n", command.c_str());
        }
        manager->loadFen(fen.c_str());
    }

    void go(chess::Manager* manager, std::istringstream& iss)
    {
        // Reset search params
        manager->impl()->search_params = chess::SearchParams();
        std::string command;
        while (iss >> command){
            if (command == "perft"){
                perft(manager, readInt(iss, "(go): Perft depth not specified or invalid\n"));
                return;
            }
            if (command == "depth"){
                manager->impl()->search_params.depth = readInt(iss, "(go): Depth not specified or invalid\n");
            }
            else if (command == "nodes"){
                manager->impl()->search_params.nodes = readInt(iss, "(go): Nodes not specified or invalid\n");
            }
            else if (command == "movetime"){
                manager->impl()->search_params.movetime = readInt(iss, "(go): Movetime not specified or invalid\n");
            }
            else if (command == "wtime"){
                manager->impl()->search_params.wtime = readInt(iss, "(go): Wtime not specified or invalid\n");
            }
            else if (command == "btime"){
                manager->impl()->search_params.btime = readInt(iss, "(go): Btime not specified or invalid\n");
            }
            else if (command == "winc"){
                manager->impl()->search_params.winc = readInt(iss, "(go): Winc not specified or invalid\n");
            }
            else if (command == "binc"){
                manager->impl()->search_params.binc = readInt(iss, "(go): Binc not specified or invalid\n");
            }
            else if (command == "ponder"){
                manager->impl()->search_params.ponder = true;
            }
            else if (command == "infinite"){
                manager->impl()->search_params.infinite = true;
            } else {
                fail("(go): Invalid command: %s\n", command.c_str());
            }
        }
        // Start the search
        manager->asyncSearch();
    }

    std::string uciReadCommImpl(chess::Manager* manager, std::string input)
    {
        std::istringstream iss(input);
        std::string command, output;
        iss >> command;

        if (command == "uci"){
            output = "id name CEngine\n";
            output += "id author IlikeChooros\n";
            output += "uciok\n";
        }
        else if (command == "ucinewgame"){
            manager->reset();
        }
        else if (command == "isready"){
            output = "readyok\n";
        }
        else if (command == "position"){
            position(manager, iss);
        }
        else if (command == "stop"){
            manager->impl()->stopSearchAsync();
        }
        else if (command == "go"){
            go(manager, iss);
        }
        else if (command == "getfen"){
            output = manager->impl()->board->getFen() + "\n";
        }
        else if (command == "help"){
            // Check if there is a command after help
            std::string help_command;
            if (iss >> help_command){
                if (help_map.find(help_command) != help_map.end()){
                    output = help_map[help_command];
                } else {
                    output = "Unknown command: '" + help_command + "', type 'help' for a list of commands\n";
                }
                return output;
            }
            // Just a help command
            output = help_map["help"];
        }
        else {
            output = "Unknown command: '" + command + "', type 'help' for a list of commands\n";
        }
        return output;
    }

    std::string uciReadCommand(chess::Manager* manager, std::string input)
    {
        std::string output;
        try{
            output = uciReadCommImpl(manager, input);
        } catch(std::exception& e){
            output = e.what();
        }
        return output;
    }

    // UCI

    UCI::UCI(): m_queue(1), m_ready(true)
    {
        m_board.init();
        m_manager = chess::Manager(&m_board);
        m_manager.init();

        std::ios_base::sync_with_stdio(false);
        std::cout.setf(std::ios::unitbuf);
    }

    UCI& UCI::operator=(UCI&& other)
    {
        m_manager = std::move(other.m_manager);
        m_board = other.m_board;
        return *this;
    }

    UCI::~UCI()
    {
        m_queue.stop();
    }

    void UCI::loop()
    {
        std::cout << "CEngine UCI\n";
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
            std::cout << uciReadCommand(&this->m_manager, comm);
        });
    }
}