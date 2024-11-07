import chess
import chess.svg
import pygame
import cairosvg
import io
from PIL import Image
from . import inputs, settings, engine

# Initialize Pygame
pygame.init()

# Create the window
window = pygame.display.set_mode(settings.WINDOW_SIZE)
pygame.display.set_caption('Chess')

# Create the board
board = chess.Board()

# Create the engine
engine_ = engine.Engine(settings.ENGINE_PATH)

# ----------------- Functions -----------------

def draw_board(board: chess.Board) -> pygame.Surface:
    """
    Return current board as a Pygame Surface,
    using the chess.svg.board function.
    """
    svg_data = chess.svg.board(
        board=board, size=settings.BOARD_SIZE, coordinates=False, 
        **inputs.render_settings.__dict__()
    ).encode('utf-8') 
    png_data = cairosvg.svg2png(bytestring=svg_data)
    image = Image.open(io.BytesIO(png_data))
    mode, size, data = image.mode, image.size, image.tobytes()
    return pygame.image.fromstring(data, size, mode)


def draw_promotion_menu() -> pygame.Surface:
    """
    Return the promotion menu as a Pygame Surface.
    """
    color: chess.Color = board.turn
    surface = pygame.Surface(settings.PROMOTION_MENU_SIZE)    

    for index, piece in enumerate(inputs.PROMOTION_PIECES):
        svg_data = chess.svg.piece(
            chess.Piece(piece, color), 
            size=settings.PROMOTION_PIECE_SIZE
        ).encode('utf-8')

        png_data = cairosvg.svg2png(bytestring=svg_data)
        image = Image.open(io.BytesIO(png_data))
        mode, size, data = image.mode, image.size, image.tobytes()
        piece_surface = pygame.image.fromstring(data, size, mode)

        surface.blit(piece_surface, (index * settings.PROMOTION_PIECE_SIZE, 0))

    return surface

# ----------------- Main loop -----------------

FPS = 30
clock = pygame.time.Clock()
running: bool = True
while running:
    # Handle events
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        else:
            inputs.handle_inputs(event, board)
    
    # Update the display at 60 FPS
    clock.tick(FPS)

    # Draw the board 
    window.blit(draw_board(board), (0, 0))

    if inputs.handle_promotion:
        window.blit(draw_promotion_menu(), settings.PROMOTION_WINDOW_OFFSETS)

    pygame.display.flip()

pygame.quit()


