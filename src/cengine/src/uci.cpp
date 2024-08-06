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
     * @brief Read input from a file
     * 
     * @param input The input file
     * @param buffer The buffer to read into
     * @param size The size of the buffer
     */
    void readInput(FILE* input, char* buffer, size_t size)
    {
        if (fgets(buffer, size, input) == NULL){
            fprintf(stderr, "Error reading input\n");
            exit(1);
        }

        // Remove newline
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n'){
            buffer[len - 1] = '\0';
        }
        if (len == 0){
            readInput(input, buffer, size);
        }
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
                if (command == "moves"){
                    break;
                }
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

        // Parse moves
        while (iss >> command){
            int from = str_to_square(command.substr(0, 2));
            int to = str_to_square(command.substr(2, 2));
            int flags = -1;

            // That's a promotion
            if (command.size() == 5){
                auto vflags = manager->getFlags(from, to);
                if (vflags.empty() || vflags.size() != 4){
                    fail("(position): Invalid promotion move: %s\n", command.c_str());
                }
                flags = vflags[0] & (Move::FLAG_PROMOTION | Move::FLAG_CAPTURE);
                flags |= Move::getPromotionPiece(command[4]);
            }

            if (!manager->movePiece(from, to, flags)){
                fail("(position): Invalid move: %s\n", command.c_str());
            }
        }
    }

    std::string uciReadCommand(chess::Manager* manager, char* input)
    {
        std::string output;
        char buffer[4096];
               
        std::istringstream iss(buffer);
        std::string command;
        iss >> command;

        if (command == "uci"){
            output = "id name CEngine\n";
            output += "id author Lucas\n";
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
        else if (command == "quit"){
            exit(0);
        }
        else if (command == "getfen"){
            output = manager->impl()->board->getFen() + "\n";
        }

        return output;
    }

    void uciLoop(FILE* input, FILE* output)
    {
        // Set the defaults and initialize the board
        chess::Board board;
        std::unique_ptr<chess::Manager> manager(new chess::Manager(&board));
        board.init();
        manager->impl()->init();

        try{
            while(1){
                char buffer[4096];
                readInput(input, buffer, sizeof(buffer));
                auto out = uciReadCommand(manager.get(), buffer);
                fprintf(output, "%s", out.c_str());
            }
        }
        catch (std::exception& e){
            fprintf(stderr, "Error: %s\n", e.what());
            fflush(output);
        }
    }

    // UCI

    UCI::UCI() {}

    void UCI::sendCommand(std::string comm)
    {
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_command = comm;
            m_ready = false;
        }        
        // Send the command to the queue
        m_queue.enqueue([this](){
            std::lock_guard<std::mutex> lock(m_mutex);
            m_result = uciReadCommand(nullptr, (char*)m_command.c_str());
        });
    }

}