#include <cengine/ui.h>

static sf::Texture pieces_texture;
static sf::Font font;
static sf::Sprite pieces_sprite;
static chess::Manager manager;
static BoardWindowState state = {InputState::None, -1};
static ui::PromotionWindow promotion = ui::PromotionWindow();

namespace ui
{
    using namespace chess;

    PromotionWindow::PromotionWindow(
        sf::Font& font, sf::Texture& texture,
        Manager* manager, int x, int y
    )
    {
        this->m_font = font;
        this->m_texture = texture;
        this->m_manager = manager;
        this->m_rect = sf::RectangleShape({4 * (120 + 10) + 2*10, 120 + 50});
        this->m_x = x - m_rect.getSize().x / 2;
        this->m_y = y;
        this->m_rect.setFillColor(sf::Color(0, 0, 0, 192));
        this->m_rect.setPosition(m_x, m_y);
    }

    PromotionWindow& PromotionWindow::operator=(PromotionWindow&& other)
    {
        this->m_font = other.m_font;
        this->m_texture = other.m_texture;
        this->m_manager = other.m_manager;
        this->m_x = other.m_x;
        this->m_y = other.m_y;
        this->m_rect = other.m_rect;
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
        text.setPosition(m_x + 10, m_y + 10);
        text.setFillColor(sf::Color::White);
        target.draw(text, states);

        sf::Sprite sprite;
        sprite.setTexture(m_texture);
        sprite.setScale(2, 2);

        int x = m_x + 10;
        const int texture_x[] = {180, 240, 120, 0}; // knight, bishop, rook, queen

        for(int i = 0; i < 4; i++){
            sprite.setTextureRect(sf::IntRect(texture_x[i], 60, 60, 60));
            sprite.setPosition(x, m_y + 40);
            target.draw(sprite, states);
            x += 120 + 10;
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
            if (m_rect.getGlobalBounds().contains(mouse.x, mouse.y)){
                // Get base flag and set the chosen piece
                auto vflags = m_manager->getFlags(state->from, state->to);
                state->move_flags = vflags[0] & (Move::FLAG_CAPTURE | Move::FLAG_PROMOTION);
                state->move_flags |= (mouse.x - m_x) / 120;
                m_manager->movePiece(state->from, state->to, state->move_flags);

                // Reset the state
                state->state = InputState::None;
                state->from = -1;
                state->move_flags = -1;
                state->current_color ^= Piece::colorMask;
            }
        }
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
        manager = Manager(&board);
        manager.init();
        manager.generateMoves();

        std::filesystem::path binary_path = std::filesystem::path(argv[0]).parent_path();
        if(!pieces_texture.loadFromFile(binary_path / "img/ChessPiecesArray.png")){
            return;
        }
        font.loadFromFile(binary_path / "font/Ubuntu-L.ttf");
        pieces_sprite.setTexture(pieces_texture, true);
        pieces_sprite.setScale(2, 2);
        promotion = PromotionWindow(font, pieces_texture, &manager, 750, 400);
        
        std::cout << "Enter side (w/b): ";
        char side;
        while(!(std::cin >> side)){
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input, enter side (w/b): ";
        }
        state.player_color = side == 'w' ? Piece::White : Piece::Black;

        
        TaskQueue queue;
        std::mutex mutex;
        bool wait_for_engine = false;
        Clock timer;
        RenderWindow window = sf::RenderWindow(sf::VideoMode(1500, 1000), "CEngine");

        while(window.isOpen()){
            Event event; 
            if(window.pollEvent(event)){
                if (event.type == Event::Closed){
                    window.close();
                    break;
                }

                // First lock the mutex
                std::unique_lock<std::mutex> lock(mutex, std::try_to_lock);
                if (!lock.owns_lock() || wait_for_engine){ // Search is running we can't handle input (modify the board/state)
                    handleInput(&manager, event, &window, &state, false);
                    continue;
                }

                if(state.state == InputState::Promote){
                    promotion.handleInput(event, &window, &state);
                } else {
                    handleInput(&manager, event, &window, &state, true);
                }

                if (manager.getSearchResult().status != GameStatus::ONGOING){
                    std::cout << "Game over\n";
                    break;
                }

                if (state.current_color != state.player_color){
                    wait_for_engine = true;
                    // Search for the best move asynchronously
                    queue.enqueue([&mutex, &wait_for_engine](){
                        std::lock_guard<std::mutex> lock(mutex);
                        manager.search();
                        manager.makeEngineMove();
                        wait_for_engine = false;
                        state.current_color ^= Piece::colorMask;
                    });
                }
            }

            if (timer.getElapsedTime().asMicroseconds() > FRAME_US){
                // Clear & redraw the window
                window.clear();
                drawBoard(window, board);
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
    void drawSquare(sf::RenderWindow& window, int r, int size, const int offset, Board* board, sf::Sprite* sprite){      
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
     * @brief Draw the selected piece moves
     * @param from The square to draw the moves from
     * @param size The size of the square (in pixels)
     * @param offset The x offset of the board (in pixels)
     * @param manager The manager to get the moves from
     */
    void drawSelectedPieceMoves(sf::RenderWindow& window, int from, int size, const int offset, Manager* manager){
        // Draw the selected square
        int x = from % 8,
            y = from / 8;
        sf::RectangleShape square({float(size), float(size)});
        square.setFillColor(sf::Color(255, 255, 0, 32));
        square.setOutlineColor(sf::Color::Yellow);
        square.setOutlineThickness(5);
        square.setPosition(float(x*size + offset), float(y*size));
        window.draw(square);

        // Draw possible moves
        square.setFillColor(sf::Color::Transparent);
        square.setOutlineThickness(5);

        auto moves = manager->getPieceMoves(from);
        for(auto& move : moves){
            sf::Color color(0, 255, 0, 128);
            if (move.flags & Move::FLAG_CAPTURE){
                color = sf::Color(255, 0, 0, 128);
            } 
            square.setOutlineColor(color);
            square.setPosition(float(move.x*size + offset), float(move.y)*size);
            window.draw(square);
        }
    }

    /**
     * @brief Draw the chess board
     * @param board The board to draw
     */
    void drawBoard(sf::RenderWindow& window, Board& board){
        // Get maximum size of the chess board
        int size, offset_x;
        getBoardSize(size, offset_x, &window);     
        // Draw the chess board
        for (int r = 0; r < 64; r++){
            drawSquare(window, r, size, offset_x, &board, &pieces_sprite);
        }

        // If there is a selected piece by the user, draw it
        if (state.state == InputState::Select){
            drawSelectedPieceMoves(window, state.from, size, offset_x, &manager);
        }
        else if (state.state == InputState::Promote){
            window.draw(promotion);
        }
    }
}