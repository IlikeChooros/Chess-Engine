"""
Script to run the main engine of the application
Can be used for any engine that supprots UCI protocol
"""

import subprocess
import chess
import dataclasses
import enum

# Custom exceptions
class EngineNotRunning(Exception):
    pass

# Dataclasses

@dataclasses.dataclass
class Evaluation:
    """
    Evaluation of a position, with a score and type
    """

    class ScoreType(enum.Enum):
        CP = 'cp'
        MATE = 'mate'
    
    score: int
    score_type: ScoreType


@dataclasses.dataclass
class SearchOptions:
    """
    Basic UCI search options
    """

    depth: int = 0
    nodes: int = 0
    movetime: int = 5000
    wtime: int = 0
    btime: int = 0
    infinite: bool = False
    ponder: bool = False

    def __str__(self) -> str:
        """
        Get the search options as a string in UCI format
        """
        items = dataclasses.asdict(self).items()
        
        formatted = []
        for key, value in items:
            if value:
                if isinstance(value, bool):
                    formatted.append(key)
                else:
                    formatted.append(f'{key} {value}')
        
        return ' '.join(formatted)


# Engine class
class Engine:

    process: subprocess.Popen
    engine_path: str
    killed: bool = False
    search_options: SearchOptions = SearchOptions()

    def __init__(self, engine_path: str) -> None:
        self.engine_path = engine_path

        self.process = subprocess.Popen(
            engine_path,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            universal_newlines=True
        )

        if not self.isready():
            raise EngineNotRunning('Engine is not responding to `isready` command')

    # Private methods
    
    def _send_command(self, command: str) -> None:
        """
        Send a command to the engine
        """
        if self.process.poll() is not None or self.killed:
            raise EngineNotRunning('Engine is not running')
        
        if command == 'quit':
            self.killed = True

        self.process.stdin.write(command + '\n')
        self.process.stdin.flush()
    
    def _read_line(self) -> str:
        """
        Read a single line from the engine output
        """
        return self.process.stdout.readline().strip()
    
    def _read_until_bestmove(self) -> str:
        """
        Read the output of the engine until the bestmove command is found

        :return: string containing the bestmove output, ex: 'bestmove e2e4'
        """
        while True:
            line = self._read_line()
            if line.startswith('bestmove'):
                return line
            

    # Public methods

    def set_position(self, board: chess.Board) -> None:
        """
        Set the position of the engine
        """
        self._send_command('position fen ' + board.fen())


    def set_position(self, moves: list[chess.Move], fen: str = chess.STARTING_FEN) -> None:
        """
        Set the position of the engine, based on a list of moves and a FEN string
        """
        self._send_command(f'position fen {fen} moves ' + ' '.join(map(str, moves)))
    

    def load_game(self, board: chess.Board, fen: str = chess.STARTING_FEN) -> None:
        """
        Load a game into the engine, based on the board's move stack and a FEN string
        """
        self.set_position(board.move_stack, fen)


    def set_search_options(self, options: SearchOptions) -> None:
        """
        Set the search options for the engine
        """
        self.search_options = options
    
    
    def get_evaluation(self):
        """
        Get the evaluation of the current position, in real-time,
        This is a generator that yields Evaluation objects
        """
        self._send_command(f'go {str(self.search_options)}')
        while True:
            line = self._read_line()
            if line.startswith('bestmove'):
                break
            
            splitted = line.split()
            if 'score' in splitted:
                score = int(splitted[splitted.index('score') + 2])
                score_type = Evaluation.ScoreType.CP if 'cp' in splitted else Evaluation.ScoreType.MATE
                yield Evaluation(score, Evaluation.ScoreType(score_type))


    def search(self) -> chess.Move:
        """
        Search for the best move in the current position

        :param options: Search options
        :return: Best move found by the engine as a chess.Move object
        """
        self._send_command(f"go {str(self.search_options)}")
        return chess.Move.from_uci(self._read_until_bestmove().split()[1])
    

    def isready(self) -> bool:
        """
        Check if the engine is ready
        """
        self._send_command('isready')
        while self._read_line() != 'readyok':
            pass
        
        return True
    

    def stop(self) -> None:
        """
        Stop the engine from searching
        """
        self._send_command('stop')
    

    def quit(self) -> None:
        """
        Quit the engine
        """
        self._send_command('quit')
        self.process.wait()
        self.process.kill()
        self.killed = True
    