"""
This srcipt reads the played games by the CEngine in training mode and analyzes them.
It load the games stored in PGN format and uses Stockfish to evaluate the moves.

Displays:
- Win / Loss / Draw ratio
- Average game length
- Losses / Wins / Draws in the opening / middle game / end game
- Average evaluation of the moves
- Average centipawn loss in the opening / middle game / end game
"""

import pathlib
import io
import time

BASE_DIR = pathlib.Path(__file__).resolve().parent.parent.parent
ENGINE_GAMEPLAY_PATH = BASE_DIR / 'src' / 'versus' / 'CEngine_v0_vs_CEngine_v0.pgn'
STOCKFISH_PATH = ''
ANALYZE_ALL_GAMES = True
SAVE_TO_FILE = True
FILE_PATH = BASE_DIR / 'src'/ 'analysis' / (ENGINE_GAMEPLAY_PATH.stem + '_analysis.txt')

analysis = {
    'total_games': 0,
    'win_loss_draw_ratio': {
        'win': 0,
        'loss': 0,
        'draw': 0
    },
    'average_game_length': 0,
    'opening': {
        'loss': 0,
        'win': 0,
        'draw': 0,
        'average_cpl': 0
    },
    'middle_game': {
        'loss': 0,
        'win': 0,
        'draw': 0,
        'average_cpl': 0
    },
    'end_game': {
        'loss': 0,
        'win': 0,
        'draw': 0,
        'average_cpl': 0
    },
    'average_evaluation': 0,
}

data = {
    'total_cpl': {
        'opening': {'cpl': 0, 'count': 0},
        'middle_game': {'cpl': 0, 'count': 0},
        'end_game': {'cpl': 0, 'count': 0}
    },
    'total_evaluation': 0,
    'total_game_length': 0
}


if __name__ == "__main__":
    try:
        if not ENGINE_GAMEPLAY_PATH or not STOCKFISH_PATH:
            # If the paths are not set, load them from the .env file
            import os
            import dotenv

            env_file = open(BASE_DIR / '.env', 'r')
            dotenv.load_dotenv(stream=env_file)

            if not ENGINE_GAMEPLAY_PATH:
                ENGINE_GAMEPLAY_PATH = os.getenv('ENGINE_GAMEPLAY_PATH')
            if not STOCKFISH_PATH:
                STOCKFISH_PATH = os.getenv('STOCKFISH_PATH')

            if not ENGINE_GAMEPLAY_PATH or not STOCKFISH_PATH:
                # Exit if the paths are not set
                raise ValueError("Error: ENGINE_GAMEPLAY_PATH and STOCKFISH_PATH must be set.")


        import stockfish
        from chess import pgn
        import numpy as np

        # Load the games
        pgn_file   = open(ENGINE_GAMEPLAY_PATH)
        engine     = stockfish.Stockfish(STOCKFISH_PATH)
        start_time = time.time()

        while True:
            game = pgn.read_game(pgn_file)

            if game is None:
                break
            
            result = game.headers['Result']
            if result == '1-0':
                analysis['win_loss_draw_ratio']['win'] += 1
            elif result == '0-1':
                analysis['win_loss_draw_ratio']['loss'] += 1
            else:
                analysis['win_loss_draw_ratio']['draw'] += 1

            analysis['total_games'] += 1
            board = game.board()
            evaluation = {'opening': np.array([]), 'middle_game': np.array([]), 'end_game': np.array([])}
            cpl = {'opening': np.array([]), 'middle_game': np.array([]), 'end_game': np.array([])}
            last_phase = 'opening'

            for move in game.mainline_moves():
                board.push(move)
                engine.set_fen_position(board.fen())
                score = engine.get_evaluation()

                if score['type'] != 'cp':
                    continue

                phase = 'opening'
                if board.fullmove_number > 10 and len(board.piece_map()) > 20:
                    phase = 'middle_game'
                elif board.fullmove_number > 20:
                    phase = 'end_game'
                
                last_phase = phase
                evaluation[phase] = np.append(evaluation[phase], score['value'])
                if len(evaluation[phase]) > 1:
                    cpl[phase] = np.append(cpl[phase], np.abs(evaluation[phase][-1] - evaluation[phase][-2]))

            # Store the evaluation of the moves
            data['total_evaluation'] += np.divide(
                np.sum(evaluation['opening']) + np.sum(evaluation['middle_game']) + np.sum(evaluation['end_game']), 100
            )
            
            # Store the centipawn loss of the moves
            for phase in np.array(['opening', 'middle_game', 'end_game']):
                if len(cpl[phase]) > 0:
                    analysis[phase]['average_cpl'] += np.divide(np.sum(cpl[phase]), 100)
                    analysis[phase]['average_cpl'] = np.divide(analysis[phase]['average_cpl'], len(cpl[phase]))
            
            data['total_game_length'] += board.fullmove_number

            # Update the win / loss / draw ratio for the opening / middle game / end game
            if result == '1-0':
                analysis[last_phase]['win'] += 1
            elif result == '0-1':
                analysis[last_phase]['loss'] += 1
            else:
                analysis[last_phase]['draw'] += 1

            print(f"{analysis['total_games']} | Avg time: {(time.time() - start_time) / analysis['total_games']}", end='\r')

            if not ANALYZE_ALL_GAMES and analysis['total_games'] >= 100:
                break


        """
        After analyzing the games, calculate the average game length, 
        win / loss / draw ratio, 
        losses / wins / draws in the opening / middle game / end game, 
        average evaluation of the moves, 
        and average centipawn loss in the opening / middle game / end game.
        """

        pgn_file.close()

        # Calculate the average evaluation of the moves
        analysis['average_evaluation'] = np.divide(data['total_evaluation'], analysis['total_games'])
        analysis['average_game_length'] = np.divide(data['total_game_length'], analysis['total_games'])

        for phase in ['opening', 'middle_game', 'end_game']:
            if data['total_cpl'][phase]['count'] != 0:
                data['total_cpl'][phase]['cpl'] = np.divide(data['total_cpl'][phase]['cpl'], data['total_cpl'][phase]['count'])

        def printAnalysis(*, file: io.FileIO | None = None):
            print("Total games: ", analysis['total_games'], file=file)
            print("Win / Loss / Draw ratio: ", analysis['win_loss_draw_ratio'], file=file)
            print(f"Average game length: {analysis['average_game_length']:.2f}", file=file)
            print(f"Average evaluation of the moves: {analysis['average_evaluation']:.2f}", file=file)

            print("Losses / Wins / Draws:", file=file)
            print("\tOpening: ", analysis['opening'], file=file)
            print("\tMiddle game: ", analysis['middle_game'], file=file)
            print("\tEnd game: ", analysis['end_game'], file=file)

            print("Average centipawn loss:", file=file)
            print(f"\tOpening: {analysis['opening']['average_cpl']:.2f}", file=file)
            print(f"\tMiddle game: {analysis['middle_game']['average_cpl']:.2f}", file=file)
            print(f"\tEnd game: {analysis['end_game']['average_cpl']:.2f}", file=file)

        printAnalysis()
        if SAVE_TO_FILE:
            with open(FILE_PATH, "w") as file:
                printAnalysis(file=file)
    
    except ImportError as e:
        print("Error: Required packages not installed: ", e)
        print("Please install the required packages: stockfish, chess, numpy, dotenv")
        print("You can install them by running:\n\tpip install stockfish chess numpy python-dotenv")
        exit(1)