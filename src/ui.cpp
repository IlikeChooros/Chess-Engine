#include "ui.h"

extern chess::Board board;

sf::RenderWindow window = sf::RenderWindow(sf::VideoMode(1500, 1000), "CEngine");


namespace ui
{
    void drawBoard();
    void drawSquare(const int& r, const int& size, const int& offset);


    void runWindow(){
        using namespace sf;
        
        window.setFramerateLimit(60);
        drawBoard();
        window.display();

        while(window.isOpen()){
            Event event; 
            if(window.pollEvent(event)){
                switch (event.type)
                {
                case Event::Closed:
                    window.close();
                    break;
                
                default:
                    break;
                }
            }
        }
    }

    

    void matchPiece(sf::Sprite& sprite, sf::Texture& texture,  const int& piece){
        int color = chess::Piece::getColor(piece);

        int offset_x = 0, offset_y = 0;

        switch(chess::Piece::getType(piece)){
            case chess::Piece::Empty:
                return;
            
            case chess::Piece::King:
                offset_x = 60;
                break;

            case chess::Piece::Bishop:
                offset_x = 240;
                break;

            case chess::Piece::Knight:
                offset_x = 180;
                break;

            case chess::Piece::Rook:
                offset_x = 120;
                break;

            case chess::Piece::Queen:
                offset_x = 0;
                break;

            default:
                offset_x = 300;
        }
        offset_y = color == chess::Piece::Black ? 0 : 60;

        sprite.setTexture(texture);
        sprite.setTextureRect(sf::IntRect(offset_x, offset_y, 60, 60));
        // sprite.setColor(color == chess::Piece::Black ? sf::Color::Black : sf::Color::White);
        sprite.setScale(2, 2);
    }

    void drawSquare(const int& r, const int& size, const int& offset, sf::Texture& texture){      
        auto LIGHT = sf::Color(158, 97, 55), DARK = sf::Color(73, 25, 25);
        int x, y;
        x = r % 8;
        y = r / 8;

        sf::Vector2f position(x*size + offset, (7-y)*size);
        sf::RectangleShape square({size, size});
        square.setFillColor(y%2 != 0 ? x%2 ? DARK : LIGHT : x%2 ? LIGHT : DARK);        
        square.setPosition(position);
        std::string str;
        str += char('A' + x);
        str += char('1' + y);

        sf::Font font;
        font.loadFromFile("Ubuntu-L.ttf");
        sf::Text coords(str, font, 16);
        coords.setPosition(position);
        coords.setFillColor(sf::Color::White);
        coords.setOutlineColor(sf::Color::White);

        sf::Sprite piece;
        matchPiece(piece, texture, board[r]);
        
        piece.setPosition(position);
        window.draw(square);
        window.draw(coords);
        window.draw(piece);
    }

    void drawBoard(){
        sf::Texture texture;
        texture.loadFromFile("src/img/ChessPiecesArray.png");
        texture.setSmooth(true);

        int size = window.getSize().y > window.getSize().x ? window.getSize().x : window.getSize().y;
        int offset_x = (window.getSize().x - size)/2;
        size /=8;        
        for (int r = 0; r < 64; r++){
            drawSquare(r, size, offset_x, texture);
        }
    }


}