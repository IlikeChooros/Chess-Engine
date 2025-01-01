"""
Project settings
"""

# Window width and height
WIDTH, HEIGHT = 900, 700
WINDOW_SIZE = (WIDTH, HEIGHT)

# Board settings
BOARD_SIZE = HEIGHT
BOARD_OFFSETS = ((WIDTH - BOARD_SIZE) // 2, 0)


# Promotion menu settings
PROMOTION_MENU_SIZE = (400, 100)
PROMOTION_PIECE_SIZE = PROMOTION_MENU_SIZE[0] // 4

# Promotion window offsets, [0] is x, [1] is y
PROMOTION_WINDOW_OFFSETS = (
    (WIDTH - PROMOTION_MENU_SIZE[0]) // 2,
    (HEIGHT - PROMOTION_MENU_SIZE[1]) // 2
)

# Evaluation settings
EVALUATION_SIZE = (BOARD_OFFSETS[0], HEIGHT)
EVALUATION_BAR_SIZE = (EVALUATION_SIZE[0] // 3, HEIGHT)
EVALUATION_BAR_OFFSETS = (EVALUATION_SIZE[0] - EVALUATION_BAR_SIZE[0], 0)
EVALUATION_BAR_OFFSETS_2 = (BOARD_OFFSETS[0] + BOARD_SIZE, 0)
EVALUATION_FONT_SIZE = 24


# Engine settings

import os
import pathlib

BASE_PATH     = pathlib.Path(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..')))
ENGINE_FOLDER = BASE_PATH / 'engines'
BIN_PATH      = BASE_PATH.parent / 'build' / 'bin'
ENGINE_PATH   = BIN_PATH / 'CEngine'