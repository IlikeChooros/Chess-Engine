#include <cengine/uci.h>

// Universal Chess Interface implementation for the CEngine
// Main refrence:
// https://backscattering.de/chess/uci/

namespace uci
{
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
        manager->search();
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