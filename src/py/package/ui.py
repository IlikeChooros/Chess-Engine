
"""

This module contains functions to draw the board and the promotion menu

"""

import io
import chess
import chess.svg
import pygame
import cairosvg
import numpy as np
import enum
import typing
from PIL import Image
from . import inputs, settings, engine


# Constants
FONT: pygame.font.Font
DARK_EVALUATION_COLOR = (26, 26, 26)
LIGHT_EVALUATION_COLOR = (216, 216, 216)
GREY_COLOR = (192, 192, 192)
DARK_COLOR = (64, 64, 64)
WHITE_COLOR = (255, 255, 255)
BLACK_COLOR = (0, 0, 0)

class WindowPath(enum.IntEnum):
    """
    Enum to represent the window path
    """
    MAIN_MENU = 0
    ANALYSIS = 1

window_path: WindowPath = WindowPath.MAIN_MENU
ui_running: bool = True

def change_window_path(path: WindowPath) -> None:
    """
    Change the window path
    """
    global window_path
    window_path = path

def change_ui_running(value: bool) -> None:
    """
    Change the UI running value
    """
    global ui_running
    ui_running = value

# Global variable to store the previous board SVG
_prev_board_svg: pygame.Surface | None = None



class BaseWidget:
    """
    Base widget class
    """

    def update(self, *args, **kwargs) -> None:
        """
        Update the widget
        """
        pass

    def draw(self, window: pygame.Surface) -> None:
        """
        Draw the widget
        """
        pass


# ----------------- Widgets -----------------

class BoardWidget(BaseWidget):
    """
    Board widget class
    """

    def __init__(self, board: chess.Board) -> None:
        self.surface = draw_board(board)

    def update(self, board: chess.Board) -> None:
        """
        Update the board widget
        """
        self.surface = draw_board(board)

    def draw(self, window: pygame.Surface) -> None:
        """
        Draw the board widget
        """
        window.blit(self.surface, settings.BOARD_OFFSETS)

class PromotionMenuWidget(BaseWidget):
    """
    Promotion menu widget class
    """

    def __init__(self, board: chess.Board) -> None:
        self.surface = draw_promotion_menu(board)

    def update(self, board: chess.Board) -> None:
        """
        Update the promotion menu widget
        """
        self.surface = draw_promotion_menu(board)

    def draw(self, window: pygame.Surface) -> None:
        """
        Draw the promotion menu widget
        """
        window.blit(self.surface, settings.PROMOTION_WINDOW_OFFSETS)

class EvaluationWidget(BaseWidget):
    """
    Evaluation widget class
    """

    def __init__(self, evaluation: engine.Evaluation | None) -> None:
        self.surface = draw_evaluation(evaluation)

    def update(self, evaluation: engine.Evaluation) -> None:
        """
        Update the evaluation widget
        """
        self.surface = draw_evaluation(evaluation)

    def draw(self, window: pygame.Surface) -> None:
        """
        Draw the evaluation widget
        """
        window.blit(self.surface, (0, 0))


class FPSWidget(BaseWidget):
    """
    FPS widget class
    """

    def __init__(self, clock: pygame.time.Clock) -> None:
        self.surface = draw_fps(clock)

    def update(self, clock: pygame.time.Clock) -> None:
        """
        Update the FPS widget
        """
        self.surface = draw_fps(clock)

    def draw(self, window: pygame.Surface) -> None:
        """
        Draw the FPS widget
        """
        window.blit(self.surface, (settings.WIDTH - settings.BOARD_OFFSETS[0], 0))

class Button(pygame.Rect):

    text: str
    callback: typing.Callable | None

    def __init__(self, text: str, position: tuple[int, int], size: tuple[int, int], callback: typing.Callable | None = None) -> None:
        super().__init__(position, size)
        self.text = text
        self.callback = callback
    
    def set_callback(self, callback: typing.Callable) -> None:
        self.callback = callback

    def call(self) -> None:
        if self.callback is not None:
            self.callback()

    def draw(self, window: pygame.Surface) -> None:
        pygame.draw.rect(window, WHITE_COLOR, self)
        text = FONT.render(self.text, True, BLACK_COLOR)
        window.blit(text, text.get_rect(center=self.center))

# ----------------- Window -----------------
class BaseWindow(BaseWidget):
    """
    Base window class
    """

    def __init__(self) -> None:
        pass

    def input(self, event: pygame.event.Event, **kwargs) -> None:
        """
        Handle the input event
        """
        pass


class AnalysisWindow(BaseWindow):
    """
    Analysis window class
    """

    def __init__(self, board: chess.Board, evaluation: engine.Evaluation, clock: pygame.time.Clock) -> None:
        self.board_widget = BoardWidget(board)
        self.evaluation_widget = EvaluationWidget(evaluation)
        self.fps_widget = FPSWidget(clock)
        self.promotion_menu_widget = PromotionMenuWidget(board)

    def update_eval(self, evaluation: engine.Evaluation) -> None:
        """
        Update the evaluation widget
        """
        self.evaluation_widget.update(evaluation)

    def update(self, board: chess.Board, evaluation: engine.Evaluation, clock: pygame.time.Clock) -> None:
        """
        Update the analysis window
        """
        self.board_widget.update(board)
        self.evaluation_widget.update(evaluation)
        self.fps_widget.update(clock)

        if inputs.handle_promotion:
            self.promotion_menu_widget.update(board)

    def draw(self, window: pygame.Surface) -> None:
        """
        Draw the analysis window
        """
        self.board_widget.draw(window)
        self.evaluation_widget.draw(window)
        self.fps_widget.draw(window)

        if inputs.handle_promotion:
            self.promotion_menu_widget.draw(window)
    
    def input(self, event: pygame.event.Event, board: chess.Board) -> None:
        """
        Handle the input event
        """
        inputs.handle_inputs(event, board)


class VersusWindow(BaseWindow):
    """
    Versus window class
    """

    def __init__(self, board: chess.Board) -> None:
        self.board_widget = BoardWidget(board)
        self.fps_widget = FPSWidget(pygame.time.Clock())

    def update(self, board: chess.Board) -> None:
        """
        Update the versus window
        """
        self.board_widget.update(board)

    def draw(self, window: pygame.Surface) -> None:
        """
        Draw the versus window
        """
        self.board_widget.draw(window)
        self.fps_widget.draw(window)
    
    def input(self, event: pygame.event.Event, board: chess.Board) -> None:
        """
        Handle the input event
        """
        inputs.handle_inputs(event, board)


# ----------------- Utility functions -----------------

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


def _eval_bar(evaluation: engine.Evaluation) -> pygame.Surface:
    """
    Return the evaluation bar as a Pygame Surface
    """ 

    eval_str: str | None = None
    clamp: float = 0.0

    # Check if the score is in centipawns or in mate
    if evaluation.score_type == engine.Evaluation.ScoreType.CP:
        clamp = eval_bar_score_clamp(evaluation.score)
        eval_str = '{:.1f}'.format(evaluation.score / 100)
    else:
        # Mate score
        clamp = 1.0 if evaluation.score > 0 else 0.0
        eval_str = f'M{abs(evaluation.score)}'

    # Set the text color and position
    text_pos = (settings.EVALUATION_BAR_SIZE[0] // 2, settings.EVALUATION_BAR_SIZE[1] - settings.EVALUATION_FONT_SIZE)
    text_color: tuple[int, int, int] = DARK_COLOR
    
    if clamp < 0.5:
        text_color = GREY_COLOR
        text_pos = (text_pos[0], settings.EVALUATION_FONT_SIZE // 2)

    # Create the surface and draw the bar
    black_height: int = settings.EVALUATION_BAR_SIZE[1] * (1 - clamp)
    surface = pygame.Surface(settings.EVALUATION_BAR_SIZE)
    surface.fill(WHITE_COLOR)
    surface.fill(DARK_EVALUATION_COLOR, (0, 0, settings.EVALUATION_BAR_SIZE[0], black_height))
    
    # Draw the text
    text = FONT.render(eval_str, True, text_color)
    surface.blit(text, text.get_rect(center=text_pos))
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
    surface.blit(_eval_bar(evaluation), settings.EVALUATION_BAR_OFFSETS)
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
        _prev_board_svg = pygame.image.fromstring(
            *get_svg_data(
                chess.svg.board(
                    board=board, size=settings.BOARD_SIZE, coordinates=False, 
                    **inputs.render_settings.__dict__()
            ))
        )

    return _prev_board_svg


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
