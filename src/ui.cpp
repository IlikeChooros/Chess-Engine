#include "ui.h"

static sf::RenderWindow window = sf::RenderWindow(sf::VideoMode(1500, 1000), "CEngine");
static sf::Texture pieces_texture;
static sf::Font font;
static sf::Sprite pieces_sprite;

namespace ui
{
    using namespace chess;

    // pre declaration
    void drawBoard(Board& board);

    void runWindow(Board& board){
        using namespace sf;

        // Constants
        constexpr int FPS = 60, FRAME_US = 1000000 / FPS;

        // Init resources
        Manager manager(&board);
        pieces_texture.loadFromFile("img/ChessPiecesArray.png");
        font.loadFromFile("font/Ubuntu-L.ttf");
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
                    // Handle input
                    handleInput(&manager, event, &window);
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

    void drawSquare(int r, int size, const int offset, Board* board, sf::Sprite* sprite){      
        static auto LIGHT = sf::Color(158, 97, 55), 
              DARK = sf::Color(73, 25, 25);
        
        int x, y;
        x = r % 8;
        y = r / 8;

        // Inverted y axis, so we need to invert the y coordinate
        sf::Vector2f position(x*size + offset, (7-y)*size);
        sf::RectangleShape square({float(size), float(size)});
        square.setFillColor(y%2 != 0 ? x%2 ? DARK : LIGHT : x%2 ? LIGHT : DARK);        
        square.setPosition(position);
        
        // Add coordinates
        const char str_coord[2] = {char('A' + x), char('1' + y)};

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

    void drawBoard(Board& board){
        // Get maximum size of the chess board
        int size, offset_x;
        getBoardSize(size, offset_x, &window);     
        for (int r = 0; r < 64; r++){
            drawSquare(r, size, offset_x, &board, &pieces_sprite);
        }
    }
}