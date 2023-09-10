#include "ui.h"

extern chess::Board board;



namespace ui
{
    void runWindow(){
        using namespace sf;
        Window window(VideoMode(500, 500), "CEngine");
        window.setFramerateLimit(60);

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

    

    std::string matchPiece(int piece){
        int color = chess::Piece::getColor(piece);

        switch(chess::Piece::getType(piece)){
            case chess::Piece::Empty:
                return " ";
            
            case chess::Piece::King:
                return color == chess::Piece::Black ? "♔" : "♚";

            case chess::Piece::Bishop:
                return color == chess::Piece::Black ? "♗" : "♝";

            case chess::Piece::Knight:
                return color == chess::Piece::Black ? "♘" : "♞" ;

            case chess::Piece::Rook:
                return color == chess::Piece::Black ? "♖" : "♜" ;

            case chess::Piece::Queen:
                return color == chess::Piece::Black ? "♕" : "♛" ;

            default:
                return color == chess::Piece::Black ? "♙" : "♟︎";
            
        }
    }

    std::string drawSquare(int r, int c){      
        return "|" + matchPiece(board[r*8 + c]) + "|";
    }

    void drawBoard(){
        std::string print;
        for (int row = 7; row >= 0; row--){
            for (int col=0; col < 8; col++){
                print += drawSquare(row, col);
            }
            print += "\n";
        }
        std::cout << print;
    }


}