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


    class IconButton: public sf::Drawable, public sf::Transformable
    {
    public:
        IconButton() = default;
        IconButton(sf::Texture& texture, sf::Vector2f pos, sf::Vector2f size, sf::IntRect texture_rect, sf::Vector2f scale = {1.0f, 1.0f});
        IconButton& operator=(const IconButton& other);
        IconButton(const IconButton& other);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
        bool isClicked(sf::Vector2i pos) const;
        void setOutline(sf::Color color, float thickness);
        sf::Vector2f getSize() const { return m_rect.getSize(); }
        sf::Vector2f getPosition() const { return m_rect.getPosition(); }
    private:
        sf::Texture m_texture;
        sf::RectangleShape m_rect;
    };

    class TextIconBtn: public sf::Drawable, public sf::Transformable
    {
    public:
        TextIconBtn() = default;
        TextIconBtn(sf::Font& font, sf::Texture& texture, sf::Vector2f pos, sf::Vector2f size, sf::IntRect texture_rect, sf::Vector2f scale = {1.0f, 1.0f});
        TextIconBtn& operator=(const TextIconBtn& other);
        TextIconBtn(const TextIconBtn& other);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
        bool isClicked(sf::Vector2i pos) const;
        void setOutline(sf::Color color, float thickness);
        void setText(std::string text);
    private:
        sf::Font m_font;
        sf::Text m_text;
        IconButton m_icon;
    };

    class EvalBar: public sf::Drawable, public sf::Transformable
    {
    public:
        EvalBar() = default;
        EvalBar(sf::Font& font, sf::Vector2f pos, sf::Vector2f size);
        EvalBar& operator=(EvalBar&& other);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
        void setEval(int eval);
    private:
        sf::Font m_font;
        sf::RectangleShape m_rect;
        sf::RectangleShape m_white_rect;
        sf::RectangleShape m_black_rect;
        sf::Text m_text;
    };

    class MoveList: public sf::Drawable, public sf::Transformable
    {
    public:
        MoveList() = default;
        MoveList(sf::Font& font, sf::Vector2f pos, sf::Vector2f size);
        MoveList& operator=(MoveList&& other);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
        void setMoves(std::vector<std::string>& moves);
    private:
        sf::Font m_font;
        sf::RectangleShape m_rect;
        std::vector<sf::Text> m_moves;
    };

    class BaseChessWindow: public sf::Drawable, public sf::Transformable
    {
    public:
        BaseChessWindow() = default;
        BaseChessWindow(sf::Font& font, sf::Texture& texture, chess::Manager* manager, sf::Vector2f pos, sf::Vector2f size);
        BaseChessWindow& operator=(BaseChessWindow&& other);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const override {}
        virtual void handleInput(sf::Event& event, sf::RenderWindow* window, BoardWindowState* state) {}
        void setManager(chess::Manager* manager) { m_manager = manager; }
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
        std::vector<IconButton> m_buttons;
    };


    class ChessboardDisplay: public BaseChessWindow
    {
    public:
        ChessboardDisplay() = default;
        ChessboardDisplay(
            sf::Font& font, sf::Texture& texture, chess::Manager* manager, 
            sf::Vector2f pos, sf::Vector2f size, BoardWindowState* state
        );
        ChessboardDisplay& operator=(ChessboardDisplay&& other);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
        void drawSquare(sf::RenderTarget& target, sf::RenderStates states, sf::Vector2f pos, float sq_size, int index) const;
        void drawSelectedPieceMoves(sf::RenderTarget& target, sf::RenderStates states, float sq_size) const;
        int getSquareIndex(sf::Vector2i pos) const;    
        void setRotate(bool rotate) { m_rotate = rotate; }
        bool getRotate() const { return m_rotate; }
    private:
        BoardWindowState* m_state;
        bool m_rotate = false;
    };

    class BoardWindow: public BaseChessWindow
    {
    public:
        BoardWindow() = default;
        BoardWindow(
            sf::Font& font, sf::Texture& texture, chess::Manager* manager, 
            sf::Vector2f pos, sf::Vector2f size, BoardWindowState* state
        );
        BoardWindow& operator=(BoardWindow&& other);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
        void setBoardState(BoardWindowState* state);
        void setManager(chess::Manager* manager) { m_boardDisplay.setManager(manager); m_promotionWindow.setManager(manager); }
        ChessboardDisplay& getBoardDisplay();
        PromotionWindow& getPromotionWindow();
    private:
        BoardWindowState* m_state;
        ChessboardDisplay m_boardDisplay;
        PromotionWindow m_promotionWindow;
    };

    class ScreenBase: public sf::Drawable, public sf::Transformable
    {
    public:
        ScreenBase() = default;
        ScreenBase(sf::Font& font, sf::Vector2f pos, sf::Vector2f size);
        ScreenBase& operator=(ScreenBase&& other);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
        virtual void handleInput(sf::Event& event, sf::RenderWindow* window, BoardWindowState* state) {}
        bool exit() const { return m_exit; }
        virtual void runOffLoop() {}
    protected:
        sf::Font m_font;
        sf::RectangleShape m_rect;
        sf::Vector2f m_pos;
        sf::Vector2f m_size;
        bool m_exit = false;
    };

    class MainMenuWindow: public ScreenBase
    {
    public:
        MainMenuWindow() = default;
        MainMenuWindow(sf::Font& font, sf::Texture& texture, sf::Vector2f pos, sf::Vector2f size);
        MainMenuWindow& operator=(MainMenuWindow&& other);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
        void handleInput(sf::Event& event, sf::RenderWindow* window, BoardWindowState* state);
    private:
        std::vector<TextIconBtn> m_buttons;
    };


    class ChessScreen: public ScreenBase
    {
    public:
        ChessScreen() = default;
        ChessScreen(
            sf::Font& font, sf::Texture& texture, sf::Vector2f pos, sf::Vector2f size, 
            chess::Manager* manager, BoardWindowState* state
        );
        ChessScreen& operator=(ChessScreen&& other);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
        virtual void handleInput(sf::Event& event, sf::RenderWindow* window, BoardWindowState* state);
        void checkGameStatus();
    protected:
        BoardWindow m_boardWindow;
        chess::Manager* m_manager;
        InputHandler m_inputHandler;
        BoardWindowState* m_state;
    };

    class PlayerVsPlayerWindow: public ChessScreen
    {
    public:
        PlayerVsPlayerWindow() = default;
        PlayerVsPlayerWindow(
                sf::Font& font, sf::Texture& texture, sf::Vector2f pos, sf::Vector2f size, 
                chess::Manager* manager, BoardWindowState* state);
        PlayerVsPlayerWindow& operator=(PlayerVsPlayerWindow&& other);
    };

    class PlayerVsEngineWindow: public ChessScreen
    {
    public:
        PlayerVsEngineWindow() = default;
        PlayerVsEngineWindow(
                sf::Font& font, sf::Texture& texture, sf::Vector2f pos, sf::Vector2f size, 
                chess::Manager* manager, BoardWindowState* state);
        PlayerVsEngineWindow& operator=(PlayerVsEngineWindow&& other);

        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
        void runOffLoop() override;
    private:
        uci::UCI m_backend;
        bool m_engine_running;
        EvalBar m_evalBar;
    };


    class AnalysisWindow: public ChessScreen
    {
        public:
        AnalysisWindow() = default;
        AnalysisWindow(
                sf::Font& font, sf::Texture& texture, sf::Vector2f pos, sf::Vector2f size, 
                chess::Manager* manager, BoardWindowState* state);
        AnalysisWindow& operator=(AnalysisWindow&& other);
        ~AnalysisWindow();

        void draw(sf::RenderTarget& target, sf::RenderStates states) const override;
        void runOffLoop() override;
    private:
        uci::UCI m_backend;
        bool m_reload;
        EvalBar m_evalBar;
        MoveList m_moveList;
        std::list<std::string> m_fens;
        sf::Clock m_clock;
    };
} // namespace ui