import multiprocessing.shared_memory
import pathlib
import chess
import chess.pgn
import time
import multiprocessing
import numpy as np
from threading import Lock
from package.engine import Engine, SearchOptions

"""

This script is used to play a match between two engines 
and save the games in a pgn file

"""

MATCH_NUMBER       = 1
SKIP_FIRST_N_LINES = 100
TOTAL_GAMES        = 200

MAX_GAMES = TOTAL_GAMES - SKIP_FIRST_N_LINES
POSTFIX = '' if MATCH_NUMBER == 1 else f'_{MATCH_NUMBER}'

# Paths
BASE_DIR = pathlib.Path(__file__).resolve().parent.parent.parent
ENGINE_WHITE_PATH = BASE_DIR / "src" / "engines" / "CEngine_v0"
ENGINE_BLACK_PATH = BASE_DIR / "src" / "engines" / "CEngine_v30"
OPENINGS_PATH = BASE_DIR / "src" / "utils" / "openings.txt"
OUTPUT_PATH = BASE_DIR / "src" / "versus" / (
    ENGINE_WHITE_PATH.stem + '_vs_' + ENGINE_BLACK_PATH.stem + POSTFIX + '.pgn'
)
OUTPUT_RESULTS_PATH = BASE_DIR / "src" / "versus" / (
    ENGINE_WHITE_PATH.stem + '_vs_' + ENGINE_BLACK_PATH.stem + POSTFIX + '_results' + '.txt'
)

SHARED_DIM = 5

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
    engine.send_command("setoption name Log File value <empty>")    
    engine.set_search_options(SEARCH_OPTIONS)

def save_game(fen: str, moves: list[chess.Move], results : list) -> tuple[int, int, int]:
    """
    Save the game in a pgn file, thread safe
    """
    global OUTPUT_PATH, ENGINE_WHITE_PATH, ENGINE_BLACK_PATH, pgn_lock

    with pgn_lock:
        # Create the game
        game = chess.pgn.Game()
        board = chess.Board(fen)
        game.setup(board)

        # Set the headers
        game.headers["Event"] = "Versus Match"
        game.headers["Site"]  = "Local"
        game.headers["Date"]  = time.strftime("%Y.%m.%d")
        game.headers["Round"] = results[0]
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
        count_lock.acquire()
        if outcome is None:
            pass
        elif outcome.winner == chess.WHITE:
            results[1] += 1
        elif outcome.winner == chess.BLACK:
            results[2] += 1
        else:
            results[3] += 1
        count_lock.release()

        # Save the results
        with open(OUTPUT_RESULTS_PATH, 'w') as results_file:
            results_file.write(f"Results:\n")
            results_file.write(f"{names['white']}: {results[1]}\n")
            results_file.write(f"{names['black']}: {results[2]}\n")
            results_file.write(f"Draw: {results[3]}\n")

        # Save the game in the pgn file
        exporter = chess.pgn.StringExporter(headers=True, variations=True, comments=True)
        with open(OUTPUT_PATH, 'a') as pgn_file:
            pgn_file.write(f"{game.accept(exporter)}\n\n")


def play_game(white: Engine, black: Engine, fen: str) -> list[chess.Move]:
    """
    Play a game between two engines, returns the moves played
    """

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
        # print(f"{len(board.move_stack)} | {resp} ", end='\r')
        
        if not board.is_valid() or board.is_game_over(claim_draw=True):
            break

    return board.move_stack


def format_time(seconds: int) -> str:
    """
    
    Format given time in seconds, to:
    *h *m *s, format where * is a number (if 0 then skipped)
    
    """
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


def play_one_game(arg : tuple[str, str]) -> None:
    """
    Play out one game between 2 engines (selected by `ENGINE_WHITE_PATH` and `ENGINE_BLACK_PATH`)
    Thread-safe, allows user to run these games in parallel
    """

    fen, shrd_name = arg

    # Init the engines
    white = Engine(ENGINE_WHITE_PATH)
    black = Engine(ENGINE_BLACK_PATH)
    setup_engine(white)
    setup_engine(black)
    
    # Play the game
    moves = play_game(white, black, fen)

    # Using shared variables in Python is waaaay to complicated than it should be
    # the `multiprocessing.Value` didn't work, so I'm using the `SharedMemory`
    existing_shm = multiprocessing.shared_memory.SharedMemory(shrd_name)
    np_array     = np.ndarray(SHARED_DIM, dtype=np.int64, buffer=existing_shm.buf)

    # Save the game (thread safe)
    value : int
    with count_lock:
        np_array[0] += 1
        value = np_array[0]

    # Save the game to file
    save_game(fen, moves, np_array)
    existing_shm.close()

    # Print the game progress
    with print_lock:
        print(
            f"{value + SKIP_FIRST_N_LINES:4} {((value)/MAX_GAMES * 100):4.1f}% | ETA: {format_time(int((time.time() - start_time)/(value) * (MAX_GAMES - value))):8} ", 
            end='\r'
        )

def create_shared_memory(np_array):
    # Create shared memory
    return multiprocessing.shared_memory.SharedMemory(create=True, size=np_array.nbytes)

def init_pool(pg_lock : Lock, prnt_lock : Lock, cnt_lock : Lock):
    """
    To avoid passing many arguments, create global variables
    `pgn_lock` 
    `print_lock`
    `count_lock`
    """
    global pgn_lock, print_lock, count_lock

    pgn_lock   = pg_lock
    print_lock = prnt_lock
    count_lock = cnt_lock


def main():
    global ENGINE_WHITE_PATH, ENGINE_BLACK_PATH, OPENINGS_PATH, \
           OUTPUT_PATH, SKIP_FIRST_N_LINES, MAX_GAMES, start_time

    # Contains fen strings of the opening positions for the training games
    openings_file = open(OPENINGS_PATH, 'r')

    # Create the output file
    try:
        open(OUTPUT_PATH, 'a')
    except FileNotFoundError:
        open(OUTPUT_PATH, 'x')

    # Set the names
    names['white'] = ENGINE_WHITE_PATH.stem
    names['black'] = ENGINE_BLACK_PATH.stem

    print("Starting the match...")
    print(f"White: {names['white']} | Black: {names['black']}")

    # Skip the first n positions    
    for _ in range(SKIP_FIRST_N_LINES):
        openings_file.readline()

    # Prepare the fens
    fens       = [openings_file.readline()[:-1] for _ in range(MAX_GAMES)]
    
    # Setup arguments for the game
    shm        = create_shared_memory(np.ones(SHARED_DIM, dtype=np.int64))
    start_time = time.time()
    print_lock = Lock()
    pgn_lock   = Lock()
    count_lock = Lock()

    args = [(fen, shm.name) for fen in fens]

    with multiprocessing.Pool(
        processes=multiprocessing.cpu_count(), 
        initializer=init_pool, 
        initargs=(pgn_lock, print_lock, count_lock)) as pool:
        pool.map(play_one_game, args)
    
    openings_file.close()
    shm.close()
    shm.unlink()

    print(f"\nMatch finished in {time.time() - start_time:.2f}s")

if __name__ == "__main__":
    main()