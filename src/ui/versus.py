import pathlib

MATCH_NUMBER = 1
SKIP_FIRST_N_LINES = 0
MAX_GAMES = 250 - SKIP_FIRST_N_LINES
POSTFIX = '' if MATCH_NUMBER == 1 else f'_{MATCH_NUMBER}'

# Paths
BASE_DIR = pathlib.Path(__file__).resolve().parent.parent
ENGINE_WHITE_PATH = BASE_DIR / "src" / "engines" / "CEngine_v0"
ENGINE_BLACK_PATH = BASE_DIR / "src" / "engines" / "CEngine_v0"
OPENINGS_PATH = BASE_DIR / "src" / "utils" / "openings.txt"
OUTPUT_PATH = BASE_DIR / "src" / "versus" / (
    ENGINE_BLACK_PATH.parent.name + '_vs_' + ENGINE_WHITE_PATH.parent.name + POSTFIX + '.pgn'
)

