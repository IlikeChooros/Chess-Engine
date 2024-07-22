#include <cengine/ui.h>

static sf::RenderWindow window = sf::RenderWindow(sf::VideoMode(1500, 1000), "CEngine");
static sf::Texture pieces_texture;
static sf::Font font;
static sf::Sprite pieces_sprite;
static chess::Manager manager;
static BoardWindowState state = {InputState::None, -1};

namespace ui
{
    using namespace chess;

    void drawBoard(Board& board);


    /**
     * @brief Main ui window loop, runs the window and handles the input
     * @param board The board to run the window with
     */
    void runWindow(Board& board, int argc, char** argv){
        using namespace sf;

        // Constants
        constexpr int FPS = 60, FRAME_US = 1000000 / FPS;

        // Init resources
        manager = Manager(&board);
        manager.generateMoves();

        std::filesystem::path binary_path = std::filesystem::path(argv[0]).parent_path();
        if(!pieces_texture.loadFromFile(binary_path / "img/ChessPiecesArray.png")){
            return;
        }
        font.loadFromFile(binary_path / "font/Ubuntu-L.ttf");
        pieces_sprite.setTexture(pieces_texture, true);
        pieces_sprite.setScale(2, 2);
        
        Clock timer;

        while(window.isOpen()){
            Event event; 
            if(window.pollEvent(event)){
                switch (event.type)
                {
                case Event::Closed:
                    window.close();
                    break;
                default:
                    handleInput(&manager, event, &window, &state);
                    break;
                }
            }
            
            if (timer.getElapsedTime().asMicroseconds() > FRAME_US){
                // Clear & redraw the window
                window.clear();
                drawBoard(board);
                window.display();
                timer.restart();
            }
        }
    }

    /**
     * @brief Match the sprite texture to the piece
     */
    void matchPiece(sf::Sprite* sprite, int piece){

        int offset_x = 0,
            offset_y = Piece::getColor(piece) == Piece::White ? 60 : 0;

        switch(Piece::getType(piece)){
            case Piece::Empty:
                return;
            case Piece::King:
                offset_x = 60;
                break;
            case Piece::Bishop:
                offset_x = 240;
                break;
            case Piece::Knight:
                offset_x = 180;
                break;
            case Piece::Rook:
                offset_x = 120;
                break;
            case Piece::Queen:
                offset_x = 0;
                break;
            default:
                offset_x = 300;
        }
        sprite->setTextureRect(sf::IntRect(offset_x, offset_y, 60, 60));
    }

    /**
     * @brief Draw a square on the board with coordinates and piece (if present)
     * @param r The index of the square
     * @param size The size of the square (in pixels)
     * @param offset The x offset of the board (in pixels)
     * @param board The board to draw the square from
     * @param sprite The sprite to draw the piece from
     */
    void drawSquare(int r, int size, const int offset, Board* board, sf::Sprite* sprite){      
        static auto LIGHT = sf::Color(158, 97, 55), 
              DARK = sf::Color(73, 25, 25);
        
        int x, y;
        x = r % 8;
        y = r / 8;

        // Inverted y axis, so we need to invert the y coordinate
        sf::Vector2f position(x*size + offset, y*size);
        sf::RectangleShape square({float(size), float(size)});
        square.setFillColor(y%2 != 0 ? x%2 ? DARK : LIGHT : x%2 ? LIGHT : DARK);        
        square.setPosition(position);
        
        // Add coordinates
        std::string str_coord = square_to_str(r);

        // Draw the coordinates
        sf::Text coords(str_coord, font, 16);
        coords.setPosition(position);
        coords.setFillColor(sf::Color::White);
        coords.setOutlineColor(sf::Color::White);        

        // Call the draw function
        window.draw(square);
        window.draw(coords);

        // If there is a piece in the square, draw it
        if (Piece::getType((*board)[r]) != Piece::Empty){
            matchPiece(sprite, (*board)[r]);
            sprite->setPosition(position);
            window.draw(*sprite);
        }
    }

    /**
     * @brief Draw the chess board
     * @param board The board to draw
     */
    void drawBoard(Board& board){
        // Get maximum size of the chess board
        int size, offset_x;
        getBoardSize(size, offset_x, &window);     
        // Draw the chess board
        for (int r = 0; r < 64; r++){
            drawSquare(r, size, offset_x, &board, &pieces_sprite);
        }

        // If there is a selected piece by the user, draw it
        if (state.state == InputState::Select){
            // Draw the selected square
            int x = state.from % 8,
                y = state.from / 8;
            sf::RectangleShape square({float(size), float(size)});
            square.setFillColor(sf::Color(255, 255, 0, 32));
            square.setOutlineColor(sf::Color::Yellow);
            square.setOutlineThickness(5);
            square.setPosition(float(x*size + offset_x), float(y*size));
            window.draw(square);

            // Draw possible moves
            square.setFillColor(sf::Color::Transparent);
            square.setOutlineThickness(5);

            auto moves = manager.getPieceMoves(state.from);
            for(auto& move : moves){
                sf::Color color(0, 255, 0, 128);
                if (move.flags & Move::FLAG_CAPTURE){
                    color = sf::Color(255, 0, 0, 128);
                } 
                square.setOutlineColor(color);
                square.setPosition(float(move.x*size + offset_x), float(move.y)*size);
                window.draw(square);
            }
        }
    }
}