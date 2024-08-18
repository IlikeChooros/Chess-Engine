#include <ui/ui.h>

static sf::Texture pieces_texture;
static sf::Font font;
static sf::Sprite pieces_sprite;
static chess::Manager *manager;
static BoardWindowState state = {};

namespace ui
{
    using namespace chess;

    IconButton::IconButton(
        sf::Texture& texture, sf::Vector2f pos, sf::Vector2f size, sf::IntRect texture_rect, sf::Vector2f scale 
    )
    {
        this->m_texture = texture;
        this->m_rect = sf::RectangleShape(size);
        this->m_rect.setPosition(pos);
        this->m_rect.setTexture(&texture);
        this->m_rect.setTextureRect(texture_rect);
        this->m_rect.setScale(scale);
    }

    IconButton::IconButton(const IconButton& other)
    {
        *this = other;
    }
    
    IconButton& IconButton::operator=(const IconButton& other)
    {
        this->m_texture = other.m_texture;
        this->m_rect = other.m_rect;
        return *this;
    }

    void IconButton::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(m_rect, states);
    }

    bool IconButton::isClicked(sf::Vector2i pos) const
    {
        return m_rect.getGlobalBounds().contains(pos.x, pos.y);
    }

    void IconButton::setOutline(sf::Color color, float thickness)
    {
        m_rect.setOutlineColor(color);
        m_rect.setOutlineThickness(thickness);
    }

    TextIconBtn::TextIconBtn(
        sf::Font& font, sf::Texture& texture, sf::Vector2f pos, sf::Vector2f size, sf::IntRect texture_rect, sf::Vector2f scale
    )
    {
        this->m_font = font;
        this->m_text = sf::Text("", font, 24);
        this->m_icon = IconButton(texture, pos, size, texture_rect, scale);
    }

    TextIconBtn::TextIconBtn(const TextIconBtn& other)
    {
        *this = other;
    }

    TextIconBtn& TextIconBtn::operator=(const TextIconBtn& other)
    {
        this->m_font = other.m_font;
        this->m_text = other.m_text;
        this->m_icon = other.m_icon;
        return *this;
    }

    void TextIconBtn::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(m_icon, states);
        target.draw(m_text, states);
    }

    bool TextIconBtn::isClicked(sf::Vector2i pos) const
    {
        return m_icon.isClicked(pos);
    }

    void TextIconBtn::setOutline(sf::Color color, float thickness)
    {
        m_icon.setOutline(color, thickness);
    }

    void TextIconBtn::setText(std::string text)
    {
        m_text.setString(text);
        m_text.setPosition(m_icon.getPosition().x + m_icon.getSize().x / 2 - m_text.getLocalBounds().width / 2, m_icon.getPosition().y + m_icon.getSize().y + 10);
    }

    EvalBar::EvalBar(sf::Font& font, sf::Vector2f pos, sf::Vector2f size)
    {
        this->m_font = font;
        this->m_rect = sf::RectangleShape({size.x - 8, size.y - 8});
        this->m_rect.setPosition({pos.x + 4, pos.y + 4});
        this->m_text = sf::Text("", font, 24);
        this->m_text.setPosition({pos.x - 30, pos.y + size.y / 2 - 12});
        this->m_text.setFillColor(sf::Color::White);
        m_rect.setFillColor(sf::Color(0, 0, 0, 255));
        m_rect.setOutlineColor(sf::Color(128, 128, 128, 255));
        m_rect.setOutlineThickness(4);
        m_white_rect.setFillColor(sf::Color(255, 255, 255, 255));
        m_black_rect.setFillColor(sf::Color(32, 32, 32, 255));
        setEval(0, true);
    }

    EvalBar& EvalBar::operator=(EvalBar&& other)
    {
        this->m_font = other.m_font;
        this->m_rect = other.m_rect;
        this->m_text = other.m_text;
        this->m_white_rect = other.m_white_rect;
        this->m_black_rect = other.m_black_rect;
        return *this;
    }

    void EvalBar::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(m_rect, states);
        target.draw(m_text, states);
        target.draw(m_black_rect, states);
        target.draw(m_white_rect, states);
    }

    void EvalBar::setEval(int eval, bool cp)
    {
        if (cp){
            char eval_s[32];
            std::snprintf(eval_s, 32, "%.02f", float(eval) / 100.0f);
            m_text.setString(eval_s);
        } else {
            std::string eval_str = eval > 0 ? "M" : "-M";
            eval_str += std::to_string(std::abs(eval));
            m_text.setString(eval_str);
        }
        
        m_text.setPosition(
            m_rect.getPosition().x - m_text.getLocalBounds().width - 15,
            m_rect.getPosition().y + m_rect.getSize().y / 2
        );

        // Normalize the eval to a value between -1 and 1
        float feval = float(eval) / 100.0f;
        float eval_f = feval / (1.0f + std::abs(float(feval)));
        float white_scale = (1.0f + eval_f) * 0.5f,
              black_scale = 1.0f - white_scale;

        if (!cp){
            white_scale = eval > 0 ? 1.0f : 0.0f;
            black_scale = 1.0f - white_scale;
        }

        // Draw black and white rectangles
        auto maxSize = m_rect.getSize();
        m_black_rect.setSize({maxSize.x, maxSize.y * black_scale});
        m_black_rect.setPosition(m_rect.getPosition());
        
        m_white_rect.setSize({maxSize.x, maxSize.y * white_scale});
        m_white_rect.setPosition({m_rect.getPosition().x, m_rect.getPosition().y + m_black_rect.getSize().y});
    }

    MoveList::MoveList(sf::Font& font, sf::Vector2f pos, sf::Vector2f size)
    {
        this->m_font = font;
        this->m_rect = sf::RectangleShape(size);
        this->m_rect.setPosition(pos);
    }

    MoveList& MoveList::operator=(MoveList&& other)
    {
        this->m_font = other.m_font;
        this->m_rect = other.m_rect;
        return *this;
    }

    void MoveList::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        sf::Text text("Engine line:", m_font, 24);
        text.setPosition(m_rect.getPosition().x + 10, m_rect.getPosition().y + 10);
        text.setFillColor(sf::Color::White);
        target.draw(text, states);
        for(size_t i = 0; i < m_moves.size(); i++){
            target.draw(m_moves[i], states);
        }
    }

    void MoveList::setMoves(std::vector<std::string>& moves)
    {
        m_moves.clear();
        for(size_t i = 0; i < moves.size(); i++){
            sf::Text text(moves[i], m_font, 24);
            text.setPosition(m_rect.getPosition().x + 10, m_rect.getPosition().y + 40 + i * 30);
            text.setFillColor(sf::Color::White);
            m_moves.push_back(text);
        }
    }


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
        const int texture_x[] = {180, 240, 120, 0}; // knight, bishop, rook, queen

        m_buttons.reserve(4);
        for(size_t i = 0; i < 4; i++){
            m_buttons.emplace_back(
                texture, sf::Vector2f{m_pos.x + (120 + 10) * i + 10, m_pos.y + 40}, 
                sf::Vector2f{120, 120}, sf::IntRect(texture_x[i], 60, 60, 60)
            );
            m_buttons.back().setOutline(sf::Color(255, 255, 255, 128), 2);
        }
    }

    PromotionWindow& PromotionWindow::operator=(PromotionWindow&& other) {
        BaseChessWindow::operator=(std::move(other));
        this->m_buttons = other.m_buttons;
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

        for(int i = 0; i < 4; i++){
            m_buttons[i].draw(target, states);
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
                if (m_buttons[i].isClicked(mouse)){
                    auto vflags = m_manager->getFlags(state->from, state->to);
                    state->move_flags = i | (vflags[0] & (Move::FLAG_CAPTURE | Move::FLAG_PROMOTION));
                    state->state = InputState::None;
                    state->current_color ^= Piece::colorMask;
                    m_manager->makeMove(state->from, state->to, state->move_flags);
                    *state->board = *m_manager->board();
                }
            }
        }
    }

    ChessboardDisplay::ChessboardDisplay(
        sf::Font& font, sf::Texture& texture,
        Manager* m, sf::Vector2f pos, sf::Vector2f size,
        BoardWindowState* state
    ): BaseChessWindow(font, texture, m, pos, size) {
        this->m_state = state;
    }

    ChessboardDisplay& ChessboardDisplay::operator=(ChessboardDisplay&& other)
    {
        BaseChessWindow::operator=(std::move(other));
        this->m_state = other.m_state;
        return *this;
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

        if (m_rotate){
            x = 7 - x;
            y = 7 - y;
        }

        // Inverted y axis, so we need to invert the y coordinate
        sf::Vector2f position(x*size + pos.x, y*size + pos.y);
        sf::RectangleShape square({size, size});
        square.setFillColor(y%2 != 0 ? x%2 == 0 ? DARK : LIGHT : x%2 == 0 ? LIGHT : DARK);        
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
        if (Piece::getType((*m_state->board)[index]) != Piece::Empty){
            sf::Sprite sprite;
            sprite.setTexture(m_texture);
            sprite.setScale(2, 2);
            matchPiece(&sprite, (*m_state->board)[index]);
            sprite.setPosition(position);
            target.draw(sprite, states);
        }
    }

    void ChessboardDisplay::drawSelectedPieceMoves(sf::RenderTarget& target, sf::RenderStates states, float size) const{
        // Draw the selected square
        int x = m_state->from % 8,
            y = m_state->from / 8;

        if (m_rotate){
            x = 7 - x;
            y = 7 - y;
        }

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
        for(auto move : moves){
            sf::Color color(0, 255, 0, 128);
            if (move.flags & Move::FLAG_CAPTURE){
                color = sf::Color(255, 0, 0, 128);
            } 
            if (m_rotate){
                move.x = 7 - move.x;
                move.y = 7 - move.y;
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

    int ChessboardDisplay::getSquareIndex(sf::Vector2i pos) const
    {
        if (!m_rect.getGlobalBounds().contains(pos.x, pos.y)){
            return -1;
        }
        float size = std::min(m_size.x, m_size.y) / 8;
        int x = (pos.x - m_pos.x) / size;
        int y = ((pos.y - m_pos.y) / size);
        return x + y*8;
    }

    BoardWindow::BoardWindow(
        sf::Font& font, sf::Texture& texture,
        Manager* manager, sf::Vector2f pos, sf::Vector2f size,
        BoardWindowState* state
    ): BaseChessWindow(font, texture, manager, pos, size)
    {
        this->m_state = state;
        this->m_boardDisplay = ChessboardDisplay(font, texture, manager, {250, 0}, {1000, 1000}, state);
        this->m_promotionWindow = PromotionWindow(font, texture, manager, {750, 500}, {120, 120});
    }

    BoardWindow& BoardWindow::operator=(BoardWindow&& other)
    {
        BaseChessWindow::operator=(std::move(other));
        this->m_boardDisplay = std::move(other.m_boardDisplay);
        this->m_promotionWindow = std::move(other.m_promotionWindow);
        this->m_state = other.m_state;
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

    ScreenBase::ScreenBase(
        sf::Font& font, sf::Vector2f pos, sf::Vector2f size
    )
    {
        this->m_font = font;
        this->m_pos = pos;
        this->m_size = size;
        this->m_rect = sf::RectangleShape(size);
        this->m_rect.setPosition(pos);
    }

    ScreenBase& ScreenBase::operator=(ScreenBase&& other)
    {
        this->m_font = other.m_font;
        this->m_pos = other.m_pos;
        this->m_rect = other.m_rect;
        this->m_size = other.m_size;
        return *this;
    }

    void ScreenBase::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(m_rect, states);
    }

    ChessScreen::ChessScreen(
        sf::Font& font, sf::Texture& texture,
        sf::Vector2f pos, sf::Vector2f size,
        Manager* manager, BoardWindowState* state
    ): ScreenBase(font, pos, size), m_inputHandler(manager, state)
    {
        m_state = state;
        m_manager = manager;

        m_rect.setFillColor(sf::Color(42, 42, 42, 192));
        m_boardWindow = BoardWindow(font, texture, manager, {250, 0}, {1000, 1000}, state);
        m_inputHandler.onPromotion([this](int index, BoardWindowState* state, sf::RenderWindow* window, sf::Event& event){
            m_boardWindow.getPromotionWindow().handleInput(event, window, state);
        });
    }

    ChessScreen& ChessScreen::operator=(ChessScreen&& other)
    {
        ScreenBase::operator=(std::move(other));
        this->m_manager = other.m_manager;
        this->m_boardWindow = std::move(other.m_boardWindow);
        return *this;
    }

    void ChessScreen::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(m_rect, states);
        target.draw(m_boardWindow, states);
    }

    void ChessScreen::handleInput(sf::Event& event, sf::RenderWindow* window, BoardWindowState* state)
    {
        int index = -1;
        if (event.type == sf::Event::MouseButtonPressed){
            auto mouse = sf::Mouse::getPosition(*window);
            index = m_boardWindow.getBoardDisplay().getSquareIndex(mouse);
            if (m_boardWindow.getBoardDisplay().getRotate())
                index = 63 - index;
        }
        m_inputHandler.handleInput(event, window, index, true);
    }

    void ChessScreen::checkGameStatus()
    {
        auto status = m_manager->getStatus();
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
        }
    }

    MainMenuWindow::MainMenuWindow(
        sf::Font& font, sf::Texture& texture,
        sf::Vector2f pos, sf::Vector2f size
    ): ScreenBase(font, pos, size)
    {
        m_rect.setFillColor(sf::Color(42, 42, 42, 192));

        // Create buttons
        m_buttons.reserve(3);
        // Player vs Player
        m_buttons.emplace_back(
            font, texture, sf::Vector2f{pos.x + 400, pos.y + 400}, 
            sf::Vector2f{200, 200}, sf::IntRect(60, 0, 60, 60)
        );
        m_buttons.back().setOutline(sf::Color(255, 255, 255, 128), 2);
        m_buttons.back().setText("Player vs Player");

        // Player vs AI
        m_buttons.emplace_back(
            font, texture, sf::Vector2f{pos.x + 400 + (50 + 200), pos.y + 400}, 
            sf::Vector2f{200, 200}, sf::IntRect(120, 0, 60, 60)
        );
        m_buttons.back().setOutline(sf::Color(255, 255, 255, 128), 2);
        m_buttons.back().setText("Player vs AI");

        // Analysis
        m_buttons.emplace_back(
            font, texture, sf::Vector2f{pos.x + 400 + (50 + 200)*2, pos.y + 400}, 
            sf::Vector2f{200, 200}, sf::IntRect(180, 0, 60, 60)
        );
        m_buttons.back().setOutline(sf::Color(255, 255, 255, 128), 2);
        m_buttons.back().setText("Analysis");
    }

    MainMenuWindow& MainMenuWindow::operator=(MainMenuWindow&& other)
    {
        ScreenBase::operator=(std::move(other));
        this->m_buttons = other.m_buttons;
        return *this;
    }

    void MainMenuWindow::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        target.draw(m_rect, states);

        sf::Text text("Main Menu", m_font, 24);
        text.setPosition(m_pos.x + m_size.x / 2 - 50, m_pos.y + 350);
        text.setFillColor(sf::Color::White);
        target.draw(text, states);

        for(size_t i = 0; i < 3; i++){
            m_buttons[i].draw(target, states);
        }
    }

    void MainMenuWindow::handleInput(sf::Event& event, sf::RenderWindow* window, BoardWindowState* state)
    {
        if(event.type == sf::Event::MouseButtonPressed){
            auto mouse = sf::Mouse::getPosition(*window);
            for(size_t i = 0; i < 3; i++){
                if (m_buttons[i].isClicked(mouse)){
                    state->screen_state = static_cast<BoardScreenState>(i + 1);
                    m_exit = true;
                    break;
                }
            }
        }
    }

    PlayerVsPlayerWindow::PlayerVsPlayerWindow(
        sf::Font& font, sf::Texture& texture,
        sf::Vector2f pos, sf::Vector2f size,
        Manager* manager, BoardWindowState* state
    ): ChessScreen(font, texture, pos, size, manager, state) {}

    PlayerVsPlayerWindow& PlayerVsPlayerWindow::operator=(PlayerVsPlayerWindow&& other)
    {
        ChessScreen::operator=(std::move(other));
        return *this;
    }

    PlayerVsEngineWindow::PlayerVsEngineWindow(
        sf::Font& font, sf::Texture& texture,
        sf::Vector2f pos, sf::Vector2f size,
        Manager* manager, BoardWindowState* state
    ): ChessScreen(font, texture, pos, size, manager, state) {
        // Allow only the player to move
        m_engine_running = false;
        state->player_color = Piece::White;
        m_manager = m_backend.getManager();
        m_boardWindow.setManager(m_manager);
        m_inputHandler.setManager(m_manager);
        *m_manager->board() = *manager->board();
        *state->board = *m_manager->board();
        m_manager->reload();

        m_inputHandler.onSelected([this](int index, BoardWindowState* state, sf::RenderWindow* window, sf::Event& event){
            if (state->player_color != state->current_color || Piece::getColor((*state->board)[index]) != state->player_color){
                return;
            }
            state->state = InputState::Select;
            state->from = index;
            state->to = -1;
            state->move_flags = -1;
        });

        m_boardWindow.getBoardDisplay().setRotate(
            state->player_color == Piece::Black
        );
        m_evalBar = EvalBar(font, {200, 0}, {50, 1000});
    }

    PlayerVsEngineWindow& PlayerVsEngineWindow::operator=(PlayerVsEngineWindow&& other)
    {
        ChessScreen::operator=(std::move(other));
        return *this;
    }

    void PlayerVsEngineWindow::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        ChessScreen::draw(target, states);
        m_evalBar.draw(target, states);
    }

    void PlayerVsEngineWindow::runOffLoop()
    {
         // Let the computer play
        if (m_state->player_color != m_state->current_color){
            if (!m_engine_running){
                m_backend.sendCommand("position fen " + m_state->board->getFen());
                m_backend.sendCommand("go depth 4 movetime 2500");
                m_engine_running = true;
            }
            else if(m_backend.commandsLeft() == 0){ // backend has finished processing all commands
                // Engine has finished searching
                m_manager->makeEngineMove();
                *m_state->board = *m_manager->board();
                m_state->current_color = m_state->player_color;
                m_engine_running = false;
            }
        }

        // Try to read the engine output
        auto& results = m_manager->getSearchResult();
        std::unique_lock<std::mutex> lock(results.mutex, std::try_to_lock);
        if (lock.owns_lock()){
            m_evalBar.setEval(results.score.value, results.score.type == Score::cp);
        }
    }

    AnalysisWindow::AnalysisWindow(
        sf::Font& font, sf::Texture& texture, sf::Vector2f pos, sf::Vector2f size, 
        chess::Manager* manager, BoardWindowState* state
    ): ChessScreen(font, texture, pos, size, manager, state)
    {
        m_manager = m_backend.getManager();
        m_boardWindow.setManager(m_manager);
        m_inputHandler.setManager(m_manager);
        m_fens.push_back(manager->board()->getFen());
        m_manager->reload();
        m_reload = true;

        m_inputHandler.onPieceMove([this](int index, BoardWindowState* state, sf::RenderWindow* window, sf::Event& event){
            if (m_manager->makeMove(state->from, state->to, state->move_flags)){
                m_reload = true;
                state->state = InputState::None;
                state->current_color ^= Piece::colorMask;
                *state->board = *m_manager->board();
                m_fens.push_back(state->board->getFen());
            }
        });

        m_inputHandler.onPromotion([this](int index, BoardWindowState* state, sf::RenderWindow* window, sf::Event& event){
            m_boardWindow.getPromotionWindow().handleInput(event, window, state);
            if (state->state == InputState::None){ // Promotion has been handled
                m_fens.push_back(state->board->getFen());
                m_reload = true;
            }
        });

        m_inputHandler.customEvent([this](BoardWindowState* state, sf::RenderWindow* window, sf::Event& event){
            if (event.type == sf::Event::KeyPressed){
                using std::chrono_literals::operator""ms;

                if (event.key.code == sf::Keyboard::R){
                    m_backend.sendCommand("stop");
                    m_backend.sendCommand("position startpos");
                    m_backend.sendCommand("ucinewgame");
                    // wait for the engine to stop
                    while(m_backend.commandsLeft() > 0) {
                        std::this_thread::sleep_for(10ms);
                    }
                    *state->board = *m_manager->board();
                    state->state = InputState::None;
                    state->current_color = Piece::White;
                    m_backend.sendCommand("go movetime 10000");
                }

                if (event.key.code == sf::Keyboard::Space){
                    m_reload = true;
                }

                if (event.key.code == sf::Keyboard::Z){
                    // Reset the board
                    if (m_fens.size() < 2){
                        return;
                    }
                    m_fens.pop_back();
                    state->board->loadFen(m_fens.back().c_str());
                    state->state = InputState::None;
                    state->current_color = state->board->getSide();
                    m_reload = true;
                }

                if (event.key.code == sf::Keyboard::O){
                    m_boardWindow.getBoardDisplay().setRotate(!m_boardWindow.getBoardDisplay().getRotate());
                }
            }
        });

        m_evalBar = EvalBar(font, {200, 0}, {50, 1000});
        m_moveList = MoveList(font, {0, 0}, {200, 1000});
    }

    AnalysisWindow::~AnalysisWindow()
    {
        m_backend.sendCommand("stop");
    }
    
    AnalysisWindow& AnalysisWindow::operator=(AnalysisWindow&& other)
    {
        ChessScreen::operator=(std::move(other));
        m_backend = std::move(other.m_backend);
        m_reload = other.m_reload;
        m_evalBar = std::move(other.m_evalBar);
        return *this;
    }

    void AnalysisWindow::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        ChessScreen::draw(target, states);
        m_evalBar.draw(target, states);
        m_moveList.draw(target, states);
    }

    void AnalysisWindow::runOffLoop()
    {
        if (m_reload)
        {
            m_backend.sendCommand("stop");
            m_backend.sendCommand("position fen " + m_state->board->getFen());
            m_backend.sendCommand("go movetime 10000");
            m_reload = false;
        }

        // Try to read the engine output
        auto& results = m_manager->getSearchResult();
        std::unique_lock<std::mutex> lock(results.mutex, std::try_to_lock);
        if (lock.owns_lock()){
            m_evalBar.setEval(results.score.value, results.score.type == Score::cp);
            std::vector<std::string> moves;
            for (auto& move : results.pv){
                moves.push_back(Piece::notation(move.getFrom(), move.getTo()));
            }
            m_moveList.setMoves(moves);
        }
    }

    EngineTesterWindow::EngineTesterWindow(
        sf::Font& font, sf::Texture& texture,
        sf::Vector2f pos, sf::Vector2f size,
        Manager* manager, BoardWindowState* state
    ): ChessScreen(font, texture, pos, size, manager, state)
    {
        m_evalBar = EvalBar(font, {200, 0}, {50, 1000});
        m_moveList = MoveList(font, {0, 0}, {200, 1000});

        m_logger.run(manager);
    }

    EngineTesterWindow& EngineTesterWindow::operator=(EngineTesterWindow&& other)
    {
        ChessScreen::operator=(std::move(other));
        return *this;
    }

    void EngineTesterWindow::runOffLoop()
    {
        if (m_clock.getElapsedTime().asMilliseconds() > 5){
            m_logger.check(manager);
            *state.board = *m_manager->board(); // update the board in UI
            m_clock.restart();
        }

        // Try to read the engine output
        auto& results = m_manager->getSearchResult();
        std::unique_lock<std::mutex> lock(results.mutex, std::try_to_lock);
        if (lock.owns_lock()){
            m_evalBar.setEval(results.score.value, results.score.type == Score::cp);
            std::vector<std::string> moves;
            for (auto& move : results.pv){
                moves.push_back(Piece::notation(move.getFrom(), move.getTo()));
            }
            m_moveList.setMoves(moves);
        }
    }

    void EngineTesterWindow::draw(sf::RenderTarget& target, sf::RenderStates states) const
    {
        ChessScreen::draw(target, states);
        m_evalBar.draw(target, states);
        m_moveList.draw(target, states);
    }

    ScreenBase* getScreen(BoardScreenState state){
        switch(state){
            case BoardScreenState::MAIN_MENU:
                return new MainMenuWindow(font, pieces_texture, {0, 0}, {1500, 1000});
            case BoardScreenState::ANALYSIS:
                return new AnalysisWindow(font, pieces_texture, {0, 0}, {1500, 1000}, manager, &::state);
            case BoardScreenState::PLAYER_VS_ENGINE:
                return new PlayerVsEngineWindow(font, pieces_texture, {0, 0}, {1500, 1000}, manager, &::state);
            case BoardScreenState::PLAYER_VS_PLAYER:
                return new PlayerVsPlayerWindow(font, pieces_texture, {0, 0}, {1500, 1000}, manager, &::state);
            case BoardScreenState::TEST_ENGINE:
                return new EngineTesterWindow(font, pieces_texture, {0, 0}, {1500, 1000}, manager, &::state);
            default:
                return new MainMenuWindow(font, pieces_texture, {0, 0}, {1500, 1000});
        }
    }


    /**
     * @brief Main ui window loop, runs the window and handles the input
     * @param board The board to run the window with
     */
    void runWindow(Board& board, int argc, char** argv){
        using namespace sf;

        // Constants
        constexpr int FPS = 60, FRAME_US = 1000000 / FPS;

        // Init resources
        Board copy(board);
        Manager m(&board);
        m.init();
        m.generateMoves();
        state.current_color = board.getSide();
        state.board = &copy;
        manager = &m;

        state.screen_state = BoardScreenState::ANALYSIS;

        if(!pieces_texture.loadFromFile(global_settings.base_path / "img/ChessPiecesArray.png")){
            return;
        }
        font.loadFromFile(global_settings.base_path / "font/Ubuntu-L.ttf");
        pieces_sprite.setTexture(pieces_texture, true);
        pieces_sprite.setScale(2, 2);

        // Print the quick guide
        std::cout << "Quick guide:\n";
        std::cout << "- Tso quit the window, press Ec\n";
        std::cout << "In analysis mode:\n";
        std::cout << "- To reset the board to initial position press 'R'\n";
        std::cout << "- To undo a move press 'Z'\n";
        std::cout << "- If something goes wrong, press 'Space' to reload the search\n";\
        std::cout << "- Press 'P' to print current FEN\n";
        std::cout << "- Press 'O' to rotate the board\n";
        std::cout << "That's it!\n";

        Clock timer;
        RenderWindow window = sf::RenderWindow(sf::VideoMode(1500, 1000), "CEngine");

        while(window.isOpen()){
            std::unique_ptr<ScreenBase> screen(getScreen(state.screen_state));
            while(!screen->exit() && window.isOpen()){
                screen->runOffLoop();
                Event event;
                while(window.pollEvent(event)){
                    if (event.type == Event::Closed || (event.type == Event::KeyPressed && event.key.code == Keyboard::Escape)){
                        window.close();
                        break;
                    }
                    screen->handleInput(event, &window, &state);
                }

                if (timer.getElapsedTime().asMicroseconds() > FRAME_US){
                    // Clear & redraw the window
                    window.clear();
                    window.draw(*screen);
                    window.display();
                    timer.restart();
                }
            }
        }
    }
}