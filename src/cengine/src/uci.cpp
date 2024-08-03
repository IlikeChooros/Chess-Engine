#include <cengine/uci.h>

// Universal Chess Interface implementation for the CEngine
// Main refrence:
// https://backscattering.de/chess/uci/

namespace uci
{

    /*

    STARTPOS 7:
    d2d4 269605607
    e2e4 309478267
    
    (expected)
    d2d4: 269605599
    e2e4: 309478263

    STARTPOS moves e2e4 (6):
    d7d5 25292264
    d7d5: 25292260

    STARTPOS moves e2e4 d7d5 (5):
    d1f3 1260402
    d1e2 809376

    d1f3: 1260400
    d1e2: 809374

    STARTPOS moves e2e4 d7d5 d1f3 (4):
    position startpos moves e2e4 d7d5 d1f3
    go perft 4
    d5e4 43885
    c8g4 42032

    d5e4: 43884
    c8g4: 42031

    STARTPOS moves e2e4 d7d5 d1f3 d5e4 (3):
    position startpos moves e2e4 d7d5 d1f3 d5e4
    go perft 3

    e1d1 1175

    e1d1: 1174

    STARTPOS moves e2e4 d7d5 d1f3 d5e4 e1d1 (2):
    position startpos moves e2e4 d7d5 d1f3 d5e4 e1d1
    go perft 2

    c8g4 25

    c8g4: 24

    STARTPOS moves e2e4 d7d5 d1f3 d5e4 e1d1 c8g4 (1):
position startpos moves e2e4 d7d5 d1f3 d5e4 e1d1 c8g4
go perft 1


    f3d3: 1



    */


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
     * @brief Run perft test at given depth, board should be already initialized
     */
    void perft(chess::Manager* manager, int depth)
    {
        test::Perft perft(manager->impl()->board);
        perft.setPrint(true);
        perft.run(depth, manager->impl()->board->getFen());
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

    void uciLoopImpl(chess::Manager* manager,FILE* input, FILE* output)
    {
        char buffer[4096];
        while (true)
        {
            readInput(input, buffer, 4096);
            
            std::istringstream iss(buffer);
            std::string command;
            iss >> command;

            if (command == "uci"){
                fprintf(output, "id name CEngine\n");
                fprintf(output, "id author Lucas\n");
                fprintf(output, "uciok\n");
            }
            else if (command == "isready"){
                fprintf(output, "readyok\n");
            }
            else if (command == "quit"){
                break;
            }
            else if (command == "ucinewgame"){
                manager->reload();
            }
            else if (command == "getfen"){
                fprintf(output, "%s\n\n", manager->impl()->board->getFen().c_str());
            }
            else if (command == "position"){
                position(manager, iss);
            }
            else if (command == "go"){
                // Check if any command follows, else search for the best move
                if(!(iss >> command)){
                    // Search for the best move
                }
                else {
                    if (command == "perft"){
                        int depth;
                        if(!(iss >> depth)){
                            fail("(perft): Depth not specified or invalid\n");
                        }
                        perft(manager, depth);
                    }
                }
            }

            fflush(output);
        }
    }

    void uciLoop(FILE* input, FILE* output)
    {
        // Set the defaults and initialize the board
        chess::Board board;
        std::unique_ptr<chess::Manager> manager(new chess::Manager(&board));
        board.init();
        manager->impl()->init();

        try{
            uciLoopImpl(manager.get(), input, output);
        }
        catch (std::exception& e){
            fprintf(stderr, "Error: %s\n", e.what());
            fflush(output);
            uciLoopImpl(manager.get(), input, output);
        }
    }
}