import multiprocessing.shared_memory
import pathlib
import chess
import chess.pgn
import time
import multiprocessing
import numpy as np
import argparse
from threading import Lock
from package.engine import Engine, SearchOptions

"""

This script is used to play a match between two engines 
and save the games in a pgn file

"""

MATCH_NUMBER       = 1
SKIP_FIRST_N_LINES = 100
TOTAL_GAMES        = 250

POSTFIX = '' if MATCH_NUMBER == 1 else f'_{MATCH_NUMBER}'

# Paths
BASE_DIR = pathlib.Path(__file__).resolve().parent.parent.parent
ENGINE_WHITE_PATH = BASE_DIR / "src" / "engines" / "CEngine_v0"
ENGINE_BLACK_PATH = BASE_DIR / "src" / "engines" / "CEngine_v31"
OPENINGS_PATH     = BASE_DIR / "src" / "utils" / "openings.txt"

# Options
BOTH_SIDES        = False
NO_PGN            = False

# For Shared Memory array
SHARED_DIM = 5
BLACK_INDEX   = 1
WHITE_INDEX   = 2
DRAW_INDEX    = 3

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
    global PGN_OUTPUT, ENGINE_WHITE_PATH, ENGINE_BLACK_PATH, pgn_lock, \
            BLACK_INDEX, WHITE_INDEX, DRAW_INDEX 
    
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
            results[WHITE_INDEX] += 1
        elif outcome.winner == chess.BLACK:
            results[BLACK_INDEX] += 1
        else:
            results[DRAW_INDEX] += 1
        count_lock.release()

        # Save the results
        with open(OUTPUT_RESULTS_PATH, 'w') as results_file:
            results_file.write(f"Results:\n")
            results_file.write(f"{names['white']}: {results[WHITE_INDEX]}\n")
            results_file.write(f"{names['black']}: {results[BLACK_INDEX]}\n")
            results_file.write(f"Draw: {results[DRAW_INDEX]}\n")

        # Save the game in the pgn file
        if not NO_PGN:
            exporter = chess.pgn.StringExporter(headers=True, variations=True, comments=True)
            with open(PGN_OUTPUT, 'a') as pgn_file:
                pgn_file.write(f"{game.accept(exporter)}\n\n")


def play_moves(white: Engine, black: Engine, fen: str) -> list[chess.Move]:
    """
    Play a game between two engines, returns the moves played
    """

    MAX_MOVE_COUNT = 170

    # Set the current engine
    board: chess.Board = chess.Board(fen)
    turn: chess.Color  = board.turn
    current_engine     = white if turn == chess.WHITE else black

    # Play the game
    while True:
        # Make a move
        current_engine.load_game(board, fen=fen)
        current_engine.go()
        resp   = current_engine.get_bestmove()
        if resp is None:
            break
        
        # Too many moves in a game, or null move
        if len(board.move_stack) > MAX_MOVE_COUNT or not bool(resp):
            raise TimeoutError

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
    global count_lock, print_lock

    fen, shrd_name = arg

    # Init the engines
    white = Engine(ENGINE_WHITE_PATH)
    black = Engine(ENGINE_BLACK_PATH)
    setup_engine(white)
    setup_engine(black)
    
    # Play the game
    save_results: bool = True
    try:
        moves = play_moves(white, black, fen)
    except:
        save_results = False

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
    if save_results:
        save_game(fen, moves, np_array)
    
    # Close the buffer
    existing_shm.close()

    # Print the game progress
    with print_lock:
        print(
            f"{value:4} {((value)/TOTAL_GAMES * 100):4.1f}% | ETA: {format_time(int((time.time() - start_time)/(value) * (TOTAL_GAMES - value))):8} ", 
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

def parse_args() -> list[str]:
    global ENGINE_BLACK_PATH, ENGINE_WHITE_PATH, \
            PGN_OUTPUT, OUTPUT_RESULTS_PATH, names, \
            OPENINGS_PATH, TOTAL_GAMES, SKIP_FIRST_N_LINES, \
            BOTH_SIDES, NO_PGN

    # Parse the arguments
    parser = argparse.ArgumentParser()
    
    parser.add_argument('--white',  '-w', 
                        help='Set path to white engine (by default will use CEngine_v0 in engines folder)', 
                        default=ENGINE_WHITE_PATH, 
                        type=lambda s: pathlib.Path(s).resolve())
    
    parser.add_argument('--black',  '-b', 
                        help='Set path to black engine (by default will use CEngine_v3 in engines folder)', 
                        default=ENGINE_BLACK_PATH, 
                        type=lambda s: pathlib.Path(s).resolve())
    
    parser.add_argument('--openings', '-p', 
                        help='''
                        Set the openings file, by default will use the one in "utils".
                        Should be a .txt with each line conatining 1 fen notation of a position
                        ''',
                        default=OPENINGS_PATH,
                        type=lambda s: pathlib.Path(s).resolve())

    parser.add_argument('--output', '-o', 
                        help='''
                        Specify the output directory, will create there 2 files with results and the pgn of each game,
                        by default the "versus" folder
                        ''',
                        default=BASE_DIR / "src" / "versus",
                        type=lambda s: pathlib.Path(s).resolve())

    parser.add_argument('--games', '-n',
                        help='Specify number of games, by default 250',
                        type=int, default=250)

    parser.add_argument('--skip', '-s',
                        help='Skip first n positions in the `openings` file, by default 0',
                        type=int, default=0)
    
    parser.add_argument('--both-sides', 
                        help='Play games with swapped sides (as a bonus), will result in 2x more games than specified',
                        default=False, action='store_true')
    
    parser.add_argument('--no-pgn', 
                        help='Wheter to create a pgn file with the games, by default pgn will be created',
                        default=False, action='store_true')

    args = parser.parse_args()

    ENGINE_WHITE_PATH  = args.white
    ENGINE_BLACK_PATH  = args.black
    OPENINGS_PATH      = args.openings
    BASE_OUTPUT        = args.output
    TOTAL_GAMES        = args.games
    SKIP_FIRST_N_LINES = args.skip
    BOTH_SIDES         = args.both_sides
    NO_PGN             = args.no_pgn
    
    print('Settings:')
    outputs = (('White', ENGINE_WHITE_PATH), ('Black', ENGINE_BLACK_PATH), ('Openings', OPENINGS_PATH),
                ('Output', BASE_OUTPUT), ('Games', TOTAL_GAMES), ('Skip', SKIP_FIRST_N_LINES), 
                ('Both Sides', BOTH_SIDES))
    
    for field, value in outputs:
        print(' %s: %s' % (field, value))

    # Check if the engine files exists
    if not (ENGINE_WHITE_PATH.exists() and ENGINE_BLACK_PATH.exists()):
        raise FileNotFoundError('Check if the paths are correctly set.')
    
    # Set the output paths (as globals)
    PGN_OUTPUT = BASE_OUTPUT / (
        ENGINE_WHITE_PATH.stem + '_vs_' + ENGINE_BLACK_PATH.stem + POSTFIX + '.pgn'
    )
    OUTPUT_RESULTS_PATH = BASE_OUTPUT / (
        ENGINE_WHITE_PATH.stem + '_vs_' + ENGINE_BLACK_PATH.stem + POSTFIX + '_results' + '.txt'
    )

    # Set the names
    names['white'] = ENGINE_WHITE_PATH.stem
    names['black'] = ENGINE_BLACK_PATH.stem

    all_fens : list[str] = []
    with open(OPENINGS_PATH, 'r') as file:
        all_fens = [line.replace('\n', '') for line in file]
        n_lines  = len(all_fens)
        SKIP_FIRST_N_LINES = int(SKIP_FIRST_N_LINES) % n_lines

        if n_lines > TOTAL_GAMES + SKIP_FIRST_N_LINES:
            return all_fens[SKIP_FIRST_N_LINES : (SKIP_FIRST_N_LINES + TOTAL_GAMES)]
        else:
            # we can't skip that many lines
            # if total games exceed the number of position ignore the skip
            if TOTAL_GAMES > n_lines:
                rep  : int = TOTAL_GAMES // n_lines
                rest : int = TOTAL_GAMES % n_lines
                return all_fens * rep + all_fens[:rest]

    return all_fens[:TOTAL_GAMES]

def main():
    global ENGINE_WHITE_PATH, ENGINE_BLACK_PATH, OPENINGS_PATH, \
           PGN_OUTPUT, OUTPUT_RESULTS_PATH, SKIP_FIRST_N_LINES, TOTAL_GAMES, \
            start_time, BOTH_SIDES, WHITE_INDEX, BLACK_INDEX

    
    # Prepare the fens
    fens       = parse_args()

    # Setup arguments for the game
    shm        = create_shared_memory(np.ones(SHARED_DIM, dtype=np.int64))
    start_time = time.time()
    print_lock = Lock()
    pgn_lock   = Lock()
    count_lock = Lock()

    args = [(fen, shm.name) for fen in fens]

    def run_pool():
        print("Starting the match...")
        print(f"White: {names['white']} | Black: {names['black']}")

        with multiprocessing.Pool(
            processes=multiprocessing.cpu_count(), 
            initializer=init_pool, 
            initargs=(pgn_lock, print_lock, count_lock)) as pool:
            pool.map(play_one_game, args)
    
    run_pool()

    # Switch the sides
    if BOTH_SIDES:
        print('\nFinnished games for one side, switching sides...')

        ENGINE_WHITE_PATH, ENGINE_BLACK_PATH = ENGINE_BLACK_PATH, ENGINE_WHITE_PATH
        BLACK_INDEX, WHITE_INDEX             = WHITE_INDEX, BLACK_INDEX

        names['white'] = ENGINE_WHITE_PATH.stem
        names['black'] = ENGINE_BLACK_PATH.stem

        start_time   = time.time()
        existing_shm = multiprocessing.shared_memory.SharedMemory(shm.name)
        np_array     = np.ndarray(SHARED_DIM, dtype=np.int64, buffer=existing_shm.buf)
        np_array[0]  = 0
        existing_shm.close()

        run_pool()

    shm.close()
    shm.unlink()

    print(f"\nMatch finished in {time.time() - start_time:.2f}s")
    print('Results written to:', OUTPUT_RESULTS_PATH, PGN_OUTPUT, sep='\n')

if __name__ == "__main__":
    main()