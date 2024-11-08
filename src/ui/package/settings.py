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
EVALUATION_FONT_SIZE = 16

# Engine settings

import os

BASE_PATH = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
ENGINE_PATH = os.path.join(BASE_PATH, 'engines', 'CEngine')