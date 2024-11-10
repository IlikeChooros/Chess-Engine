"""
Script to handle the inputs of the user (mouse and keyboard)
"""

from typing import Any
import pygame
import chess
import chess.svg
import numpy as np
from . import settings
import dataclasses

@dataclasses.dataclass
class BoardRenderSettings:
    """
    Rendering settings for the board
    """
    
    fill: dict = dataclasses.field(default_factory=dict)
    lastmove: chess.Move | None = None
    check: chess.Color | None = None
    orientation: chess.Color = chess.WHITE

    def __dict__(self) -> dict:
        return {
            'fill': self.fill,
            'lastmove': self.lastmove,
            'check': self.check,
            'orientation': self.orientation
        }

    def __str__(self) -> str:
        return str(self.__dict__())

def default_move_callback() -> None:
    pass

render_settings = BoardRenderSettings() 
_prev_render_settings = BoardRenderSettings()
handle_promotion: bool = False
PROMOTION_PIECES = (chess.QUEEN, chess.ROOK, chess.BISHOP, chess.KNIGHT)
make_move_callback = default_move_callback
_last_square: chess.Square | None = None
_selected_square: chess.Square | None = None


def render_settings_diff() -> bool:
    """
    Return the difference between the current and previous render settings
    """
    return dataclasses.asdict(render_settings) != dataclasses.asdict(_prev_render_settings)


def update_render_settings() -> None:
    """
    Update the previous render settings
    """
    global _prev_render_settings
    _prev_render_settings = dataclasses.replace(render_settings)


def is_out_of_bounds(pos: tuple[int, int], box: tuple[int, int]) -> bool:
    """
    Check if a position is out of bounds

    :param pos: Position to check
    :return: True if the position is out of bounds
    """
    return pos[1] < 0 or pos[1] > box[1] or pos[0] < 0 or pos[0] > box[0]

def _make_move(board: chess.Board, move: chess.Move) -> None:
    """
    Make a move on the board and update the rendering settings.
    Used internally by `handle_inputs`.

    :param board: Chess board instance
    :param move: Move to make
    """
    global _last_square, _selected_square, render_settings

    # reset the last square and fill
    _last_square = None
    _selected_square = None
    render_settings.fill = {}

    # Check if the move is legal
    if move in board.legal_moves:
        board.push(move)
        render_settings.lastmove = move
        make_move_callback()


def handle_promotion_menu(event: pygame.event.Event, board: chess.Board) -> None:
    """
    Handle the promotion menu events.

    :param event: Event to handle
    """
    global handle_promotion

    if event.type == pygame.MOUSEBUTTONDOWN:
        print(f'Promotion menu clicked at {event.pos}')
        pos = np.array(event.pos) - np.array(settings.PROMOTION_WINDOW_OFFSETS)

        # Out of bounds
        if is_out_of_bounds(pos, settings.PROMOTION_MENU_SIZE):
            return
        
        # Get the piece selected
        handle_promotion = False
        promotion_piece = PROMOTION_PIECES[pos[0] // settings.PROMOTION_PIECE_SIZE]

        _make_move(board, chess.Move(_last_square, _selected_square, promotion=promotion_piece))
    

def handle_inputs(event: pygame.event.Event, board: chess.Board) -> None:
    """
    Handle the inputs of the user (mouse and keyboard),
    set's the rendering options in `render_settings` and
    updates the board accordingly.

    :param event: Event to handle
    :param board: Chess board instance
    """
    global _selected_square, _last_square, render_settings, handle_promotion, _prev_render_settings

    if event.type == pygame.MOUSEBUTTONDOWN:
        
        update_render_settings()

        # Handle promotion menu if it's active
        if handle_promotion:
            handle_promotion_menu(event, board)
            return

        # Get the position of the mouse and normalize it to the board coordinates
        pos = (np.array(event.pos) - np.array(settings.BOARD_OFFSETS))

        # Out of bounds
        if is_out_of_bounds(pos, (settings.BOARD_SIZE, settings.BOARD_SIZE)):
            return

        # Normalize the position to the board coordinates
        pos = pos // (settings.BOARD_SIZE // 8)

        # Set y position to 7 - y if orientation is white
        y_pos = 7 - pos[1] if render_settings.orientation == chess.WHITE else pos[1]
        _selected_square = chess.square(pos[0], y_pos)

        # Make the move if two squares are selected
        if _last_square is None:
            _last_square = _selected_square
            render_settings.fill = {_selected_square: '#cdd16a'} # Highlight the selected square
            
            # Highlight legal moves
            for move in board.generate_legal_moves(chess.Bitboard(1 << _selected_square)):
                render_settings.fill[move.to_square] = '#aaa23b'
        else:
            # set the move
            move = chess.Move(_last_square, _selected_square)
 
            # Check if the move is a promotion
            legal_moves = list(board.generate_legal_moves(
                chess.Bitboard(1 << _last_square), chess.Bitboard(1 << _selected_square)
            ))

            # If there are legal moves and the first move is a promotion 
            # (then any legal move is a promotion)
            if len(legal_moves) > 0 and legal_moves[0].promotion is not None:
                handle_promotion = True
                return

            _make_move(board, move)
    
    if event.type == pygame.KEYDOWN:
        print(f'Key pressed: {pygame.key.name(event.key)}')