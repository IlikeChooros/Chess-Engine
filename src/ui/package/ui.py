
"""

This module contains functions to draw the board and the promotion menu

"""

import io
import chess
import chess.svg
import pygame
import cairosvg
import numpy as np
from PIL import Image
from . import inputs, settings, engine

FONT: pygame.font.Font
DARK_EVALUATION_COLOR = (26, 26, 26)
LIGHT_EVALUATION_COLOR = (216, 216, 216)
GREY_COLOR = (192, 192, 192)
DARK_COLOR = (64, 64, 64)
WHITE_COLOR = (255, 255, 255)
BLACK_COLOR = (0, 0, 0)

_prev_board_svg: tuple[bytes, tuple[int, int], str] | None = None

def init() -> None:
    """
    Initialize the Pygame font
    """
    pygame.init()
    pygame.font.init()
    
    global FONT
    FONT = pygame.font.Font(None, settings.EVALUATION_FONT_SIZE)


def get_svg_data(data: str) -> tuple[bytes, tuple[int, int], str]:
    """
    Return the PNG data, the size of the image and the mode of the image
    """
    png_data = cairosvg.svg2png(bytestring=data.encode('utf-8'))
    image = Image.open(io.BytesIO(png_data))
    return image.tobytes(), image.size, image.mode


def eval_bar_score_clamp(score: int) -> float:
    """
    Clamp the score to the range [0, 1]
    """
    fscore: float = score / 400
    return (fscore / (1 + np.abs(fscore))) * 0.5 + 0.5


def _eval_bar(score: int) -> pygame.Surface:
    """
    Return the evaluation bar as a Pygame Surface
    """
    clamp = eval_bar_score_clamp(score)
    black_height: int = settings.EVALUATION_BAR_SIZE[1] * (1 - clamp)
    surface = pygame.Surface(settings.EVALUATION_BAR_SIZE)
    surface.fill(WHITE_COLOR)
    surface.fill(DARK_EVALUATION_COLOR, (0, 0, settings.EVALUATION_BAR_SIZE[0], black_height))
    
    text_color = GREY_COLOR if clamp < 0.5 else DARK_COLOR
    text = FONT.render(str(score), True, text_color)
    surface.blit(text, text.get_rect(center=(settings.EVALUATION_BAR_SIZE[0] // 2, settings.EVALUATION_BAR_SIZE[1] // 2)))
    return surface


def draw_evaluation(evaluation: engine.Evaluation) -> pygame.Surface:
    """
    Return the evaluation as a Pygame Surface
    """
    surface = pygame.Surface(settings.EVALUATION_SIZE)

    # Draw the principal variation
    for index, move in enumerate(evaluation.pv):
        text = FONT.render(str(move), True, WHITE_COLOR)
        surface.blit(text, (0, index * settings.EVALUATION_FONT_SIZE))

    # Draw the eval bar
    surface.blit(_eval_bar(evaluation.score), settings.EVALUATION_BAR_OFFSETS)
    return surface


def draw_board(board: chess.Board) -> pygame.Surface:
    """
    Return current board as a Pygame Surface,
    using the chess.svg.board function.
    """

    global _prev_board_svg

    # Check if the board has changed or if the previous board is None
    if inputs.render_settings_diff() or _prev_board_svg is None:
        inputs.update_render_settings()
        _prev_board_svg = get_svg_data(
            chess.svg.board(
                board=board, size=settings.BOARD_SIZE, coordinates=False, 
                **inputs.render_settings.__dict__()
        ))

    return pygame.image.fromstring(*_prev_board_svg)


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


def draw_fps(clock: pygame.time.Clock) -> pygame.Surface:
    """
    Return the FPS as a Pygame Surface
    """

    surface = pygame.Surface((settings.BOARD_OFFSETS[0], 50))
    surface.fill(BLACK_COLOR)
    fps = FONT.render(f'FPS: {clock.get_fps():.1f}', True, WHITE_COLOR)
    surface.blit(fps, fps.get_rect(center=(settings.BOARD_OFFSETS[0] // 2, settings.EVALUATION_FONT_SIZE // 2)))
    return surface