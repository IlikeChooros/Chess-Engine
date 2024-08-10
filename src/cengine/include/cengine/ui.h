#pragma once

#include <iostream>
#include <SFML/Graphics.hpp>
#include <filesystem>

#include "manager.h"
#include "input_handle.h"
#include "threads.h"
#include "uci.h"


namespace ui
{
    void runWindow(chess::Board& board, int argc, char** argv);

    class BaseChessWindow: public sf::Drawable, public sf::Transformable
    {
    public:
        BaseChessWindow() = default;
        BaseChessWindow(sf::Font& font, sf::Texture& texture, chess::Manager* manager, sf::Vector2f pos, sf::Vector2f size);
        BaseChessWindow& operator=(BaseChessWindow&& other);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const override {}
        virtual void handleInput(sf::Event& event, sf::RenderWindow* window, BoardWindowState* state) {}
    protected:
        sf::Font m_font;
        sf::Texture m_texture;
        sf::RectangleShape m_rect;
        chess::Manager* m_manager;
        sf::Vector2f m_pos;
        sf::Vector2f m_size;
    };

    class PromotionWindow: public BaseChessWindow
    {
    public:
        PromotionWindow() = default;
        PromotionWindow(sf::Font& font, sf::Texture& texture, chess::Manager* manager, sf::Vector2f pos, sf::Vector2f size);
        PromotionWindow& operator=(PromotionWindow&& other);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
        void handleInput(sf::Event& event, sf::RenderWindow* window, BoardWindowState* state);
    private:
        sf::RectangleShape m_promoRects[4];
    };


    class ChessboardDisplay: public BaseChessWindow
    {
    public:
        ChessboardDisplay() = default;
        ChessboardDisplay(sf::Font& font, sf::Texture& texture, chess::Manager* manager, sf::Vector2f pos, sf::Vector2f size);
        ChessboardDisplay& operator=(ChessboardDisplay&& other);

        void setState(BoardWindowState* state);
        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
        void drawSquare(sf::RenderTarget& target, sf::RenderStates states, sf::Vector2f pos, float sq_size, int index) const;
        void drawSelectedPieceMoves(sf::RenderTarget& target, sf::RenderStates states, float sq_size) const;
        int getSquareIndex(sf::Vector2f pos) const;    
    private:
        BoardWindowState* m_state;
    };

    class BoardWindow: public BaseChessWindow
    {
    public:
        BoardWindow() = default;
        BoardWindow(sf::Font& font, sf::Texture& texture, chess::Manager* manager, sf::Vector2f pos, sf::Vector2f size);
        BoardWindow& operator=(BoardWindow&& other);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
        void setBoardState(BoardWindowState* state);
        ChessboardDisplay& getBoardDisplay();
        PromotionWindow& getPromotionWindow();
    private:
        BoardWindowState* m_state;
        ChessboardDisplay m_boardDisplay;
        PromotionWindow m_promotionWindow;
    };


    class MainMenuWindow: public BaseChessWindow
    {
    public:
        MainMenuWindow() = default;
        MainMenuWindow(sf::Font& font, sf::Texture& texture, chess::Manager* manager, sf::Vector2f pos, sf::Vector2f size);
        MainMenuWindow& operator=(MainMenuWindow&& other);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
    };
} // namespace ui