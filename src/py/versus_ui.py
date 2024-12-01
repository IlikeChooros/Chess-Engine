import pygame
import chess
import typing

from package import engine
from package import ui
from package import settings


pygame.init()

pygame.display.set_caption('Chess Engine')
window = pygame.display.set_mode(settings.WINDOW_SIZE)

board = chess.Board()
white = engine.Engine(settings.ENGINE_PATH, should_print=True)
black = engine.Engine(settings.ENGINE_PATH, should_print=True)

SEARCH_OPTIONS = engine.SearchOptions(movetime=1000)
FEN = chess.STARTING_FEN

white.set_search_options(SEARCH_OPTIONS)
black.set_search_options(SEARCH_OPTIONS)

white.set_position(fen=FEN)
black.set_position(fen=FEN)

white_generator = white.get_evaluation()
black_generator = black.get_evaluation()

def update_eval(engine_: engine.Engine, evaluation_generator: typing.Generator[engine.Evaluation, None, None]) -> None:
    """
    Update the evaluation of the current position, this is a hook for the make_move_callback
    Whenever a move is made, the evaluation is updated by this function
    """

    engine_.stop()
    engine_.isready()
    engine_.load_game(board, fen=settings.STARTING_FEN)

    if evaluation_generator is not None:
        evaluation_generator.close()

    evaluation_generator = engine_.get_evaluation()

def try_make_move(board: chess.Board, move: chess.Move) -> None:
    """
    Make a move on the board and update the rendering settings.
    Used internally by `handle_inputs`.

    :param board: Chess board instance
    :param move: Move to make
    """

    if move in board.legal_moves:
        board.push(move)
        update_eval(white, white_generator)
        update_eval(black, black_generator)

ui_window = ui.VersusWindow(board)

clock = pygame.time.Clock()
while True:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            pygame.quit()
            exit()

    ui_window.draw(window)

    pygame.display.flip()
    clock.tick(60)


pygame.quit()