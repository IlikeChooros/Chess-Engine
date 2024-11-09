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
pygame.display.set_caption('Chess')

# Create the board
board = chess.Board()

# Create the engine
engine_ = engine.Engine(settings.ENGINE_PATH)
search_options = engine.SearchOptions(
    depth=6, movetime=1000
)
engine_.set_search_options(search_options)

# ----------------- Callbacks -----------------

evaluation_generator: typing.Generator[engine.Evaluation, None, None] | None = None

def update_eval() -> None:
    engine_.stop()
    engine_.isready()
    engine_.load_game(board)

    global evaluation_generator

    if evaluation_generator is not None:
        evaluation_generator.close()
    
    evaluation_generator = engine_.get_evaluation()

inputs.make_move_callback = update_eval
evaluation: engine.Evaluation | None = None

# ----------------- Main loop -----------------

FPS = 60
clock = pygame.time.Clock()
running: bool = True

while running:
    # Handle events
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        else:
            inputs.handle_inputs(event, board)

    # Draw the board 
    window.blit(ui.draw_board(board), settings.BOARD_OFFSETS)

    if inputs.handle_promotion:
        window.blit(ui.draw_promotion_menu(board), settings.PROMOTION_WINDOW_OFFSETS)

    # Update the evaluation
    if evaluation_generator is not None:
        try:
            evaluation = next(evaluation_generator)
        except StopIteration:
            evaluation_generator = None

    if evaluation is not None:
      window.blit(ui.draw_evaluation(evaluation), (0, 0))
    
    # Draw the FPS
    window.blit(ui.draw_fps(clock), (settings.WIDTH - settings.BOARD_OFFSETS[0], 0))

    # Update the window
    pygame.display.flip()

    # Update the clock
    clock.tick(FPS)

pygame.quit()


