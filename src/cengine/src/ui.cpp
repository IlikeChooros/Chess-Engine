#include <cengine/ui.h>

static sf::Texture pieces_texture;
static sf::Font font;
static sf::Sprite pieces_sprite;
static chess::Manager *manager;
static BoardWindowState state = {};

namespace ui
{
    using namespace chess;

    BaseChessWindow::BaseChessWindow(
        sf::Font& font, sf::Texture& texture,
        Manager* manager, sf::Vector2f pos, sf::Vector2f size
    )
    {
        this->m_font = font;
        this->m_texture = texture;
        this->m_manager = manager;
        this->m_pos = pos;
        this->m_size = size;
        this->m_rect = sf::RectangleShape(size);
        this->m_rect.setPosition(pos);
    }

    BaseChessWindow& BaseChessWindow::operator=(
        BaseChessWindow&& other
    )
    {
        this->m_font = other.m_font;
        this->m_texture = other.m_texture;
        this->m_manager = other.m_manager;
        this->m_pos = other.m_pos;
        this->m_rect = other.m_rect;
        this->m_size = other.m_size;
        return *this;
    }

    PromotionWindow::PromotionWindow(
        sf::Font& font, sf::Texture& texture,
        Manager* manager, sf::Vector2f pos, sf::Vector2f size
    ): BaseChessWindow(font, texture, manager, pos, size)
    {
        this->m_rect = sf::RectangleShape({4 * (120 + 10) + 2*10, 120 + 50});
        this->m_pos.x = pos.x - m_rect.getSize().x / 2;
        this->m_pos.y = pos.y;
        this->m_rect.setFillColor(sf::Color(0, 0, 0, 192));
        this->m_rect.setPosition(m_pos);

        for(size_t i = 0; i < 4; i++){
            m_promoRects[i] = sf::RectangleShape({120, 120});
            m_promoRects[i].setPosition(m_pos.x + (120 + 10) * i + 10, m_pos.y + 40);
            m_promoRects[i].setOutlineColor(sf::Color(255, 255, 255, 128));
            m_promoRects[i].setOutlineThickness(2);
            m_promoRects[i].setFillColor(sf::Color(0, 0, 0, 196));
        }
    }

    PromotionWindow& PromotionWindow::operator=(PromotionWindow&& other) {
        BaseChessWindow::operator=(std::move(other));
        for(size_t i = 0; i < 4; i++){
            m_promoRects[i] = other.m_promoRects[i];
        }
        return *this;
    }

    /**
     * @brief Draw the promotion window
     * @param target The target to draw the window to
     * @param states The render states
     */
    void PromotionWindow::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(m_rect, states);

        sf::Text text("Promote to:", m_font, 24);
        text.setPosition(m_pos.x + 10, m_pos.y + 10);
        text.setFillColor(sf::Color::White);
        target.draw(text, states);

        sf::Sprite sprite;
        sprite.setTexture(m_texture);
        sprite.setScale(2, 2);

        const int texture_x[] = {180, 240, 120, 0}; // knight, bishop, rook, queen

        for(int i = 0; i < 4; i++){
            target.draw(m_promoRects[i], states);
            sprite.setTextureRect(sf::IntRect(texture_x[i], 60, 60, 60));
            sprite.setPosition(m_promoRects[i].getPosition());
            target.draw(sprite, states);
        }
    }

    /**
     * @brief Handle input for the promotion window
     * @param event The event to handle
     * @param window The window to get the input from
     * @param state The state to update
     */
    void PromotionWindow::handleInput(sf::Event& event, sf::RenderWindow* window, BoardWindowState* state)
    {
        if(event.type == sf::Event::MouseButtonPressed){
            auto mouse = sf::Mouse::getPosition(*window);

            for(size_t i = 0; i < 4; i++){
                if (m_promoRects[i].getGlobalBounds().contains(mouse.x, mouse.y)){
                    auto vflags = m_manager->getFlags(state->from, state->to);
                    state->move_flags = i | (vflags[0] & (Move::FLAG_CAPTURE | Move::FLAG_PROMOTION));
                    state->state = InputState::None;
                    state->current_color ^= Piece::colorMask;
                    m_manager->movePiece(state->from, state->to, state->move_flags);
                    *state->board = *m_manager->board();
                }
            }
        }
    }

    ChessboardDisplay::ChessboardDisplay(
        sf::Font& font, sf::Texture& texture,
        Manager* m, sf::Vector2f pos, sf::Vector2f size
    ): BaseChessWindow(font, texture, m, pos, size) {
        this->m_state = nullptr;
    }

    ChessboardDisplay& ChessboardDisplay::operator=(ChessboardDisplay&& other)
    {
        BaseChessWindow::operator=(std::move(other));
        this->m_state = other.m_state;
        return *this;
    }

    void ChessboardDisplay::setState(BoardWindowState* state)
    {
        this->m_state = state;
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

    void ChessboardDisplay::drawSquare(
        sf::RenderTarget& target, sf::RenderStates states,
        sf::Vector2f pos, float size, int index
    ) const
    {
        static auto LIGHT = sf::Color(158, 97, 55), 
              DARK = sf::Color(73, 25, 25);
        
        int x, y;
        x = index % 8;
        y = index / 8;

        // Inverted y axis, so we need to invert the y coordinate
        sf::Vector2f position(x*size + pos.x, y*size + pos.y);
        sf::RectangleShape square({size, size});
        square.setFillColor(y%2 != 0 ? x%2 ? DARK : LIGHT : x%2 ? LIGHT : DARK);        
        square.setPosition(position);
        
        // Add coordinates
        std::string str_coord = square_to_str(index);

        // Draw the coordinates
        sf::Text coords(str_coord, m_font, 16);
        coords.setPosition(position);
        coords.setFillColor(sf::Color::White);
        coords.setOutlineColor(sf::Color::White);        

        // Draw the square
        target.draw(square, states);
        target.draw(coords, states);

        // If there is a piece in the square, draw it
        if (Piece::getType((*m_manager->board())[index]) != Piece::Empty){
            sf::Sprite sprite;
            sprite.setTexture(m_texture);
            sprite.setScale(2, 2);
            matchPiece(&sprite, (*m_manager->board())[index]);
            sprite.setPosition(position);
            target.draw(sprite, states);
        }
    }

    void ChessboardDisplay::drawSelectedPieceMoves(sf::RenderTarget& target, sf::RenderStates states, float size) const{
        // Draw the selected square
        int x = m_state->from % 8,
            y = m_state->from / 8;
        sf::RectangleShape square({size, size});
        square.setFillColor(sf::Color(255, 255, 0, 32));
        square.setOutlineColor(sf::Color::Yellow);
        square.setOutlineThickness(5);
        square.setPosition(float(x*size + m_pos.x), float(y*size));
        target.draw(square, states);

        // Draw possible moves
        square.setFillColor(sf::Color::Transparent);
        square.setOutlineThickness(5);

        auto moves = m_manager->getPieceMoves(m_state->from);
        for(auto& move : moves){
            sf::Color color(0, 255, 0, 128);
            if (move.flags & Move::FLAG_CAPTURE){
                color = sf::Color(255, 0, 0, 128);
            } 
            square.setOutlineColor(color);
            square.setPosition(float(move.x*size + m_pos.x), float(move.y)*size);
            target.draw(square, states);
        }
    }

    void ChessboardDisplay::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        float sq_size = std::min(m_size.x, m_size.y) / 8;
        // Draw the chess board
        for (int r = 0; r < 64; r++){
            this->drawSquare(target, states, m_pos, sq_size, r);
        }

        // If there is a selected piece by the user, draw it
        if (m_state && m_state->state == InputState::Select){
            this->drawSelectedPieceMoves(target, states, sq_size);
        }
    }

    int ChessboardDisplay::getSquareIndex(sf::Vector2f pos) const
    {
        if (!m_rect.getGlobalBounds().contains(pos)){
            return -1;
        }
        float size = std::min(m_size.x, m_size.y) / 8;
        int x = (pos.x - m_pos.x) / size;
        int y = (pos.y - m_pos.y) / size;
        return x + y*8;
    }

    BoardWindow::BoardWindow(
        sf::Font& font, sf::Texture& texture,
        Manager* manager, sf::Vector2f pos, sf::Vector2f size
    ): BaseChessWindow(font, texture, manager, pos, size)
    {
        this->m_boardDisplay = ChessboardDisplay(font, texture, manager, {250, 0}, {1000, 1000});
        this->m_promotionWindow = PromotionWindow(font, texture, manager, {750, 500}, {120, 120});
    }

    BoardWindow& BoardWindow::operator=(BoardWindow&& other)
    {
        BaseChessWindow::operator=(std::move(other));
        this->m_boardDisplay = std::move(other.m_boardDisplay);
        this->m_promotionWindow = std::move(other.m_promotionWindow);
        return *this;
    }

    void BoardWindow::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        m_boardDisplay.draw(target, states);
        if (m_state && m_state->state == InputState::Promote){
            m_promotionWindow.draw(target, states);
        }
    }

    void BoardWindow::setBoardState(BoardWindowState* state)
    {
        m_boardDisplay.setState(state);
        m_state = state;
    }

    ChessboardDisplay& BoardWindow::getBoardDisplay()
    {
        return m_boardDisplay;
    }

    PromotionWindow& BoardWindow::getPromotionWindow()
    {
        return m_promotionWindow;
    }

    void drawBoard(sf::RenderWindow& window, Board& board);


    /**
     * @brief Main ui window loop, runs the window and handles the input
     * @param board The board to run the window with
     */
    void runWindow(Board& board, int argc, char** argv){
        using namespace sf;

        // Constants
        constexpr int FPS = 60, FRAME_US = 1000000 / FPS;

        // Init resources
        uci::UCI backend;
        state.current_color = board.getSide();
        manager = backend.getManager();
        manager->board()->loadFen(board.getFen().c_str());
        manager->reload();
        state.board = &board;

        if(!pieces_texture.loadFromFile(global_settings.base_path / "img/ChessPiecesArray.png")){
            return;
        }
        font.loadFromFile(global_settings.base_path / "font/Ubuntu-L.ttf");
        pieces_sprite.setTexture(pieces_texture, true);
        pieces_sprite.setScale(2, 2);
        BoardWindow board_window(font, pieces_texture, manager, {0, 0}, {1500, 1000});
        board_window.setBoardState(&state);
        
        std::cout << "Enter side (w/b): ";
        char side;
        while(!(std::cin >> side)){
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input, enter side (w/b): ";
        }
        state.player_color = side == 'w' ? Piece::White : Piece::Black;

        bool running = true;
        bool engine_running = false;
        Clock timer;
        RenderWindow window = sf::RenderWindow(sf::VideoMode(1500, 1000), "CEngine");

        while(window.isOpen()){

            if (state.player_color != state.current_color){
                if (!engine_running){
                    backend.sendCommand("position fen " + state.board->getFen());
                    backend.sendCommand("go depth 2 movetime 2500");
                    engine_running = true;
                }
                else if(backend.commandsLeft() == 0){ // backend has finished processing all commands
                    // Engine has finished searching
                    manager->makeEngineMove();
                    *state.board = *manager->board();
                    state.current_color = state.player_color;
                    engine_running = false;
                }
            }
            
            Event event; 
            if(window.pollEvent(event)){
                if (event.type == Event::Closed){
                    window.close();
                    break;
                }

                // if (manager->searchRunning()){ // Search is running we can't handle input (modify the board/state)
                //     handleInput(manager, event, &window, &state, false);
                //     continue;
                // }

                if(state.state == InputState::Promote){
                    board_window.getPromotionWindow().handleInput(event, &window, &state);
                } else {
                    handleInput(manager, event, &window, &state, running);
                }

                if (!running){
                    continue;
                }

                auto status = manager->getStatus();
                if (status != GameStatus::ONGOING){
                    std::cout << "Game over\n";
                    std::cout << state.board->getFen() << "\n";
                    std::string msg;
                    
                    if (status == GameStatus::CHECKMATE){
                        msg = "Checkmate, ";
                        msg += (state.board->getSide() == Piece::White ? "Black" : "White");
                        msg += " wins!";
                    } else if (status == GameStatus::STALEMATE){
                        msg = "Stalemate!";
                    } else {
                        msg = "Draw!";
                    }

                    std::cout << msg << "\n";

                    running = false;
                    backend.sendCommand("stop");
                    continue;
                }

            }

            if (timer.getElapsedTime().asMicroseconds() > FRAME_US){
                // Clear & redraw the window
                window.clear();
                // drawBoard(window, *state.board);
                window.draw(board_window);
                window.display();
                timer.restart();
            }
        }
    }
}