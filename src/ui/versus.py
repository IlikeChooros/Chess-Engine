import pathlib
import chess
import chess.pgn
import time
from package.engine import Engine


MATCH_NUMBER = 1
SKIP_FIRST_N_LINES = 0
MAX_GAMES = 250 - SKIP_FIRST_N_LINES
POSTFIX = '' if MATCH_NUMBER == 1 else f'_{MATCH_NUMBER}'

# Paths
BASE_DIR = pathlib.Path(__file__).resolve().parent.parent.parent
ENGINE_WHITE_PATH = BASE_DIR / "src" / "engines" / "CEngine_v0"
ENGINE_BLACK_PATH = BASE_DIR / "src" / "engines" / "CEngine_v0"
OPENINGS_PATH = BASE_DIR / "src" / "utils" / "openings.txt"
OUTPUT_PATH = BASE_DIR / "src" / "versus" / (
    ENGINE_BLACK_PATH.stem + '_vs_' + ENGINE_WHITE_PATH.stem + POSTFIX + '.pgn'
)


def main():
    global ENGINE_WHITE_PATH
    global ENGINE_BLACK_PATH
    global OPENINGS_PATH

    # Contains fen strings of the opening positions for the training games
    openings_file = open(OPENINGS_PATH, 'r')
    pgn_file = None

    try:
        pgn_file = open(OUTPUT_PATH, 'a')
    except FileNotFoundError:
        pgn_file = open(OUTPUT_PATH, 'xt')

    white = Engine(ENGINE_WHITE_PATH)
    black = Engine(ENGINE_BLACK_PATH)
    current_engine = white

    def runGame(engine: Engine, moves: list[chess.Move | None]) -> bool:
        """
        Function to make a move with the engine. 
        Returns False if the game is over.
        """
        engine.send_command("go movetime 100")
        resp = engine.get_bestmove()
        moves.append(resp)
        return resp is not None

    # Skip the first n positions
    for _ in range(SKIP_FIRST_N_LINES):
        openings_file.readline()
    
    print("Starting the match...")
    for i in range(MAX_GAMES):

        # Load the opening position
        fen = openings_file.readline().strip()
        
        # Set the current engine
        turn: chess.Color = chess.Board(fen=fen).turn
        if turn == chess.BLACK:
            current_engine = black

        white.send_command("ucinewgame")
        black.send_command("ucinewgame")

        white.set_position(fen)
        black.set_position(fen)
        moves: list[str | None] = []

        # Play the game
        while runGame(current_engine, moves):
            if turn == chess.WHITE:
                current_engine = white
            else:
                current_engine = black
            turn = not turn
            current_engine.set_position(moves, fen=fen)

        # Save the game in a pgn file
        moves.pop(-1) # Remove the last move (it's None)
        game = chess.pgn.Game()
        board = chess.Board(fen)
        game.setup(board)

        game.headers["Event"] = "Versus Match"
        game.headers["Site"]  = "Local"
        game.headers["Date"]  = time.strftime("%Y.%m.%d")
        game.headers["Round"] = i + 1
        game.headers["White"] = white.name
        game.headers["Black"] = black.name

        node = game
        for move in moves:
            node = node.add_variation(move)
            board.push(node.move)

        game.headers["Result"] = board.result(claim_draw=True)
        exporter = chess.pgn.StringExporter(headers=True, variations=True, comments=True)
        pgn_file.write(f"{game.accept(exporter)}\n\n")

        print(f"Completed: {((i + 1)/MAX_GAMES * 100):.1f}%")
    
    pgn_file.close()
    openings_file.close()

if __name__ == "__main__":
    main()