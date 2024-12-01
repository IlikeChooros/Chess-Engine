import chess
import pygame
import typing

import package.ui as ui
import package.inputs as inputs
import package.settings as settings
import package.engine as engine


# Initialize Pygame
ui.init()

# Create the window
window = pygame.display.set_mode(settings.WINDOW_SIZE)
pygame.display.set_caption('Chess Engine')

# Create the board
STARTING_FEN: str = '8/5pk1/6p1/8/4Q3/8/5K2/8 w - - 0 1'
board = chess.Board()
board.set_fen(STARTING_FEN)

# Create the engine
engine_ = engine.Engine(settings.ENGINE_PATH, should_print=True)
engine_.send_command('setoption name Log File value <empty>')
search_options = engine.SearchOptions(
    movetime=10000
)
engine_.set_search_options(search_options)
engine_.set_position(board)

# ----------------- Widgets -----------------

clock = pygame.time.Clock()
evaluation = engine.Evaluation()

ui_window = ui.AnalysisWindow(board, evaluation, clock)

# ----------------- Callbacks -----------------

evaluation_generator: typing.Generator[engine.Evaluation, None, None] | None = None

def update_eval() -> None:
    """
    Update the evaluation of the current position, this is a hook for the make_move_callback
    Whenever a move is made, the evaluation is updated by this function
    """

    engine_.stop()
    engine_.isready()
    engine_.load_game(board, fen=STARTING_FEN)

    # Update the evaluation generator
    global evaluation_generator

    if evaluation_generator is not None:
        evaluation_generator.close()
    
    evaluation_generator = engine_.get_evaluation()

inputs.make_move_callback = update_eval

# ----------------- Main loop -----------------

FPS = 60
evaluation_generator = engine_.get_evaluation() # Start the evaluation generator

while ui.ui_running:
    # Handle events
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            ui.ui_running = False
        else:
            ui_window.input(event, board)

    # Update the evaluation
    if evaluation_generator is not None:
        try:
            evaluation = next(evaluation_generator)
        except StopIteration:
            evaluation_generator = None

    # Update the window
    ui_window.update(board, evaluation, clock)
    ui_window.draw(window)

    # Update the window
    pygame.display.flip()

    # Update the clock
    clock.tick(FPS)

pygame.quit()


