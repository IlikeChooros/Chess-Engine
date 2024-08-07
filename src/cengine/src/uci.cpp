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
        manager->impl()->board->loadFen(fen.c_str());
        manager->reload();
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
        else if (command == "go"){
            // Check if any command follows, else search for the best move
            if(!(iss >> command)){
                // Search for the best move
                command = "search";
            }

            if (command == "perft"){
                int depth;
                if(!(iss >> depth)){
                    fail("(perft): Depth not specified or invalid\n");
                }
                perft(manager, depth);
            }
            else if (command == "search"){
                manager->search();
                auto result = manager->getSearchResult();
                output = (result.move.move() != 0 ? 
                    chess::Piece::notation(result.move.getFrom(), result.move.getTo()) : "0000"
                ) + "\n";
            }
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
        m_manager = chess::Manager(&m_board);
        m_manager.init();
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
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_command = comm;
            m_ready = false;
        }
        // Send the command to the queue
        m_queue.enqueue([this](){
            std::lock_guard<std::mutex> lock(this->m_mutex);
            std::cout << uciReadCommand(&this->m_manager, this->m_command);
            this->m_ready = true;
        });
    }

    /**
     * @brief Wheter given task has ended
     */
    bool UCI::isReady()
    {
        std::unique_lock<std::mutex> l(m_mutex, std::try_to_lock);
        return l.owns_lock() && m_ready && !m_result.empty();
    }

    /**
     * @brief Return the result of the command, first you should check if the
     * UCI is ready.
     */
    std::string UCI::getResult()
    {
        if (!isReady())
            return "";
        std::lock_guard<std::mutex> l(m_mutex);
        auto ret = m_result.front();
        m_result.pop();
        return ret;
    }
}