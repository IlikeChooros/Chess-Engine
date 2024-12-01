import pathlib
import chess
import chess.pgn
import time
from package.engine import Engine, SearchOptions


"""

This script is used to play a match between two engines 
and save the games in a pgn file

"""

MATCH_NUMBER       = 1
SKIP_FIRST_N_LINES = 0
TOTAL_GAMES        = 100

MAX_GAMES = TOTAL_GAMES - SKIP_FIRST_N_LINES
POSTFIX = '' if MATCH_NUMBER == 1 else f'_{MATCH_NUMBER}'

# Paths
BASE_DIR = pathlib.Path(__file__).resolve().parent.parent.parent
ENGINE_WHITE_PATH = BASE_DIR / "src" / "engines" / "CEngine_v0"
ENGINE_BLACK_PATH = BASE_DIR / "src" / "engines" / "CEngine_v0"
OPENINGS_PATH = BASE_DIR / "src" / "utils" / "openings.txt"
OUTPUT_PATH = BASE_DIR / "src" / "versus" / (
    ENGINE_BLACK_PATH.stem + '_vs_' + ENGINE_WHITE_PATH.stem + POSTFIX + '.pgn'
)
OUTPUT_RESULTS_PATH = BASE_DIR / "src" / "versus" / (
    ENGINE_BLACK_PATH.stem + '_vs_' + ENGINE_WHITE_PATH.stem + POSTFIX + '_results' + '.txt'
)

results: dict = {
    'white': 0,
    'black': 0,
    'draw': 0
}

names: dict = {
    'white': '',
    'black': ''
}

SEARCH_OPTIONS = SearchOptions(movetime=200)

def setup_engine(engine: Engine):
    """
    Setup the engine, turn off the logging and set the hash size
    """
    engine.send_command("uci")
    engine.isready()
    engine.send_command("setoption name Hash value 64")

    if engine.name == 'CEngine_v0':
        engine.send_command("setoption name Log File value <empty>")
    else:
        engine.send_command("setoption name Log File value log.txt")
    engine.set_search_options(SEARCH_OPTIONS)

def save_game(fen: str, moves: list[chess.Move], game_number: int):
    """
    Save the game in a pgn file
    """
    global OUTPUT_PATH, ENGINE_WHITE_PATH, ENGINE_BLACK_PATH

    # Create the game
    game = chess.pgn.Game()
    board = chess.Board(fen)
    game.setup(board)

    # Set the headers
    game.headers["Event"] = "Versus Match"
    game.headers["Site"]  = "Local"
    game.headers["Date"]  = time.strftime("%Y.%m.%d")
    game.headers["Round"] = game_number
    game.headers["White"] = ENGINE_WHITE_PATH.stem
    game.headers["Black"] = ENGINE_BLACK_PATH.stem

    # Add the moves
    node = game
    for move in moves:
        node = node.add_variation(move)
        board.push(node.move)

    # Set the result
    game.headers["Result"] = board.result(claim_draw=True)

    outcome = board.outcome(claim_draw=True)
    if outcome is None:
        pass
    elif outcome.winner == chess.WHITE:
        results['white'] += 1
    elif outcome.winner == chess.BLACK:
        results['black'] += 1
    else:
        results['draw'] += 1

    # Save the results
    with open(OUTPUT_RESULTS_PATH, 'w') as results_file:
        results_file.write(f"Results:\n")
        results_file.write(f"{names['white']}: {results['white']}\n")
        results_file.write(f"{names['black']}: {results['black']}\n")
        results_file.write(f"Draw: {results['draw']}\n")

    # Save the game in the pgn file
    exporter = chess.pgn.StringExporter(headers=True, variations=True, comments=True)
    with open(OUTPUT_PATH, 'a') as pgn_file:
        pgn_file.write(f"{game.accept(exporter)}\n\n")


def play_game(white: Engine, black: Engine, fen: str) -> list[chess.Move]:
    """
    Play a game between two engines, returns the moves played
    """

    # Set the engines
    white.stop()
    white.isready()
    black.stop()
    black.isready()
    white.send_command("ucinewgame")
    black.send_command("ucinewgame")

    # Set the current engine
    board: chess.Board = chess.Board(fen)
    turn: chess.Color  = board.turn
    current_engine     = white if turn == chess.WHITE else black

    # Play the game
    while True:
        # Make a move
        current_engine.load_game(board, fen=fen)
        current_engine.go()
        resp = current_engine.get_bestmove()
        if resp is None or not bool(resp):
            break
        
        # Switch the turn and the engine
        board.push(resp)
        turn = not turn
        current_engine = white if turn == chess.WHITE else black
        print(f"{len(board.move_stack)} | {resp} ", end='\r')
        
        if not board.is_valid() or board.is_game_over(claim_draw=True):
            # print(f"Undetected game over: {board.result(claim_draw=True)} {board.status()}")
            # print(f"FEN: {fen}", f"moves: {[str(move) for move in moves]}", sep='\n')
            # input('Press enter to continue...')
            break

    return board.move_stack


def format_time(seconds: int) -> str:
    secs: int = seconds % 60
    mins: int = seconds // 60
    hour: int = mins // 60
    
    format_ = ''
    if hour:
        format_ += f"{hour}h "
    if mins:
        format_ += f"{mins}m "
    format_ += f"{secs}s "

    return format_

def main():
    global ENGINE_WHITE_PATH, ENGINE_BLACK_PATH, OPENINGS_PATH, \
           OUTPUT_PATH, SKIP_FIRST_N_LINES, MAX_GAMES

    # Contains fen strings of the opening positions for the training games
    openings_file = open(OPENINGS_PATH, 'r')

    # Create the output file
    try:
        open(OUTPUT_PATH, 'a')
    except FileNotFoundError:
        open(OUTPUT_PATH, 'x')

    # Set the engines
    white = Engine(ENGINE_WHITE_PATH)
    black = Engine(ENGINE_BLACK_PATH)
    setup_engine(white)
    setup_engine(black)

    names['white'] = white.name
    names['black'] = black.name

    # Skip the first n positions
    for _ in range(SKIP_FIRST_N_LINES):
        openings_file.readline()
    
    print("Starting the match...")

    # Play the games
    start_time = time.time()
    for i in range(MAX_GAMES):

        # Load the opening position
        fen = openings_file.readline().strip()
        
        # Play the game
        moves = play_game(white, black, fen)

        # Save the game
        save_game(fen, moves, i + 1 + SKIP_FIRST_N_LINES)

        print(
            f"{i + 1 + SKIP_FIRST_N_LINES:15} {((i + 1)/MAX_GAMES * 100):4.1f}% | ETA: {format_time(int((time.time() - start_time)/(i + 1) * (MAX_GAMES - i - 1))):8} ", 
            end='\r'
        )
    
    openings_file.close()

    print(f"Match finished in {time.time() - start_time:.2f}s")

if __name__ == "__main__":
    main()