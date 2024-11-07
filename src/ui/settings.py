"""
Project settings
"""

# Window width and height
WIDTH, HEIGHT = 700, 700
BOARD_SIZE = HEIGHT

PROMOTION_MENU_SIZE = (400, 100)
PROMOTION_PIECE_SIZE = PROMOTION_MENU_SIZE[0] // 4

# Promotion window offsets, [0] is x, [1] is y
PROMOTION_WINDOW_OFFSETS = (
    (WIDTH - PROMOTION_MENU_SIZE[0]) // 2,
    (HEIGHT - PROMOTION_MENU_SIZE[1]) // 2
)

# Window size
WINDOW_SIZE = (WIDTH, HEIGHT)
