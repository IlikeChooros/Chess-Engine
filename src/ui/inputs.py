"""
Script to handle the inputs of the user (mouse and keyboard)
"""

from typing import Any
import pygame
import chess
import chess.svg
import numpy as np
from . import settings


class BoardRenderSettings:
    """
    Rendering settings for the board
    """
    
    fill: dict = {}
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


render_settings = BoardRenderSettings() 
handle_promotion: bool = False
promotion_piece: chess.PieceType | None = None
PROMOTION_PIECES = (chess.QUEEN, chess.ROOK, chess.BISHOP, chess.KNIGHT)
_last_square: chess.Square | None = None


def handle_promotion_menu(event: pygame.event.Event) -> None:
    """
    Handle the promotion menu events.
    If successful, sets `promotion_piece` to the selected piece.

    :param event: Event to handle
    """
    global promotion_piece, handle_promotion

    if event.type == pygame.MOUSEBUTTONDOWN:
        print(f'Promotion menu clicked at {event.pos}')
        pos = np.array(event.pos) - np.array(settings.PROMOTION_WINDOW_OFFSETS)
        print(f'Normalized position: {pos}')
        # Out of bounds
        if pos[1] < 0 or pos[1] > settings.PROMOTION_MENU_SIZE[1]:
            return
        
        if pos[0] < 0 or pos[0] > settings.PROMOTION_MENU_SIZE[0]:
            return
        
        # Get the piece selected
        promotion_piece = PROMOTION_PIECES[pos[0] // settings.PROMOTION_PIECE_SIZE]


def handle_inputs(event: pygame.event.Event, board: chess.Board) -> None:
    """
    Handle the inputs of the user (mouse and keyboard),
    set's the rendering options in `render_settings` and
    updates the board accordingly.

    :param event: Event to handle
    :param board: Chess board instance
    """
    global _last_square, render_settings, handle_promotion, promotion_piece

    if event.type == pygame.MOUSEBUTTONDOWN:
        
        # Handle promotion menu if it's active
        if handle_promotion:
            handle_promotion_menu(event)
            return

        # Get the position of the mouse and normalize it to the board coordinates
        pos = np.array(event.pos) // (settings.BOARD_SIZE // 8)
        # Set y position to 7 - y if orientation is white
        y_pos = 7 - pos[1] if render_settings.orientation == chess.WHITE else pos[1]
        square = chess.square(pos[0], y_pos)

        # Make the move if two squares are selected
        if _last_square is None:
            _last_square = square
            render_settings.fill = {square: '#cdd16a'} # Highlight the selected square
            
            # Highlight legal moves
            for move in board.generate_legal_moves(chess.Bitboard(1 << square)):
                render_settings.fill[move.to_square] = '#aaa23b'
        else:

            # set the move
            move = chess.Move(_last_square, square)

            # Check promotion was handled, then set promotion flag
            if promotion_piece is not None:
                move = chess.Move(_last_square, square, promotion=promotion_piece)
                promotion_piece = None
            else:
                # Check if the move is a promotion
                legal_moves = list(board.generate_legal_moves(
                    chess.Bitboard(1 << _last_square), chess.Bitboard(1 << square)
                ))
                # If there are legal moves and the first move is a promotion 
                # (then any legal move is a promotion)
                if len(legal_moves) > 0 and legal_moves[0].promotion is not None:
                    handle_promotion = True
                    promotion_piece = legal_moves[0].promotion
                    return

            # reset the last square and fill
            _last_square = None
            render_settings.fill = {}

            # Check if the move is legal
            if move in board.legal_moves:
                board.push(move)
                render_settings.lastmove = move
    
    if event.type == pygame.KEYDOWN:
        print(f'Key pressed: {pygame.key.name(event.key)}')