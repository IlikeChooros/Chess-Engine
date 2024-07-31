#pragma once

#include <iostream>
#include <SFML/Graphics.hpp>
#include <filesystem>

#include "manager.h"
#include "input_handle.h"


namespace ui
{
    void runWindow(chess::Board& board, int argc, char** argv);

    class PromotionWindow: public sf::Drawable, public sf::Transformable
    {
    public:
        PromotionWindow() = default;
        PromotionWindow(sf::Font& font, sf::Texture& texture, chess::Manager* manager, int x, int y);
        PromotionWindow& operator=(PromotionWindow&& other);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
        void handleInput(sf::Event& event, sf::RenderWindow* window, BoardWindowState* state);
    private:
        sf::Font m_font;
        sf::Texture m_texture;
        sf::RectangleShape m_rect;
        chess::Manager* m_manager;
        int m_x, m_y;
    };
}