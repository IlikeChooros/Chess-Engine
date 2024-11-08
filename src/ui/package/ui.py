
"""

This module contains functions to draw the board and the promotion menu

"""

import io
import chess
import chess.svg
import pygame
import cairosvg
from PIL import Image
from . import inputs, settings, engine

def get_svg_data(data: str) -> tuple[bytes, tuple[int, int], str]:
    """
    Return the PNG data, the size of the image and the mode of the image
    """
    png_data = cairosvg.svg2png(bytestring=data.encode('utf-8'))
    image = Image.open(io.BytesIO(png_data))
    return image.tobytes(), image.size, image.mode

def draw_evaluation(evaluation: engine.Evaluation) -> pygame.Surface:
    """
    Return the evaluation as a Pygame Surface
    """
    surface = pygame.Surface(settings.EVALUATION_SIZE)
    font = pygame.font.Font(None, settings.EVALUATION_FONT_SIZE)
    text = font.render('HELLO', True, (0, 0, 0))
    surface.blit(text, (0, 0))

    # Draw the principal variation
    # for index, move in enumerate(evaluation.pv):
    #     text = font.render(str(move), True, (0, 0, 0))
    #     surface.blit(text, (0, (index + 1) * settings.EVALUATION_FONT_SIZE))

    return surface

def draw_board(board: chess.Board) -> pygame.Surface:
    """
    Return current board as a Pygame Surface,
    using the chess.svg.board function.
    """
    return pygame.image.fromstring(
        *get_svg_data(
            chess.svg.board(
                board=board, size=settings.BOARD_SIZE, coordinates=False, 
                **inputs.render_settings.__dict__()
            )))


def draw_promotion_menu(board: chess.Board) -> pygame.Surface:
    """
    Return the promotion menu as a Pygame Surface.
    """
    color: chess.Color = board.turn
    surface = pygame.Surface(settings.PROMOTION_MENU_SIZE)    

    for index, piece in enumerate(inputs.PROMOTION_PIECES):

        piece_surface = pygame.image.fromstring(
            *get_svg_data(
                chess.svg.piece(
                    chess.Piece(piece, color), 
                    size=settings.PROMOTION_PIECE_SIZE
                )))

        surface.blit(piece_surface, (index * settings.PROMOTION_PIECE_SIZE, 0))

    return surface
