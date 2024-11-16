"""
Script to run the main engine of the application
Can be used for any engine that supprots UCI protocol
"""

import subprocess
import chess
import dataclasses
import enum
import typing
import select
import pathlib

# Custom exceptions
class EngineNotRunning(Exception):
    """
    Exception raised when the engine is not running
    """
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
    
    score: int = 0
    score_type: ScoreType = ScoreType.CP
    nps: int = 0
    pv: list[chess.Move] = dataclasses.field(default_factory=list)

    def _from_string(self, string: str) -> None:
        """
        Set the Evaluation from a uci engine output string

        ex. `info depth 1 seldepth 1 score cp 0 nps 0 time 0 nodes 0 pv e2e4`
        """
        splitted = string.split()
        
        # Read the score, type and nodes per second
        self.score = int(splitted[splitted.index('score') + 2])
        self.score_type = Evaluation.ScoreType.CP if 'cp' in splitted else Evaluation.ScoreType.MATE
        self.nps = int(splitted[splitted.index('nps') + 1])

        # Read the principal variation
        self.pv.clear()
        if 'pv' not in splitted:
            return

        pv_index: int = splitted.index('pv')
        while True:
            try:
                self.pv.append(chess.Move.from_uci(splitted[pv_index + 1]))
                pv_index += 1
            except (IndexError, chess.InvalidMoveError):
                break



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
    name: str = 'Engine'
    should_print: bool = False

    def __init__(self, engine_path: str, should_print = False) -> None:
        self.engine_path = engine_path
        self.name = pathlib.Path(engine_path).stem
        self.should_print = should_print

        self.process = subprocess.Popen(
            engine_path,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            universal_newlines=True
        )

        if not self.isready():
            raise EngineNotRunning(self._format_error('Engine is not ready'))

    # Private methods
    
    def _format_error(self, error: str) -> str:
        """
        Format an error message
        """
        return '[%s] %s' % (self.name, error)
    
    def _read_line(self) -> str:
        """
        Read a single line from the engine output
        """
        out = self.process.stdout.readline().strip()

        if self.should_print:
            print('[%s]: %s' % (self.name, out))
        return out
    
    def _try_read_line(self) -> str | None:
        """
        Try to read a single line from the engine output
        """
        if select.select([self.process.stdout], [], [], 0)[0]:
            return self._read_line()
        return None
    
    def _read_until_bestmove(self) -> chess.Move | None:
        """
        Read the output of the engine until the bestmove command is found.

        This is used to get the best move after a search command

        :return: string containing the bestmove output, ex: 'bestmove e2e4'
        """
        while True:
            line = self._read_line()
            if line.startswith('bestmove'):
                strmove = line.split()[1]
                move: chess.Move | None = None
                if strmove != '(none)':
                    move = chess.Move.from_uci(strmove)
                return move
            

    # Public methods

    def send_command(self, command: str) -> None:
        """
        Send a command to the engine
        """
        if self.process.poll() is not None or self.killed:
            raise EngineNotRunning(self._format_error('Engine is not ready'))
        
        if command == 'quit':
            self.killed = True

        if self.should_print:
            print(command)
        
        self.process.stdin.write(command + '\n')
        self.process.stdin.flush()

    @typing.overload
    def set_position(self, board: chess.Board) -> None: 
        """
        Set the position of the engine, based on a chess.Board object
        """
        ...

    @typing.overload
    def set_position(self, moves: list[chess.Move], fen: str = chess.STARTING_FEN) -> None:
        """
        Set the position of the engine, based on a list of moves and a FEN string
        """
        ...
    

    def set_position(self, *args, **kwargs) -> None:
        # Implement the overloads
        if len(args) == 1:
            if isinstance(args[0], chess.Board):
                self.send_command('position fen ' + args[0].fen())
            elif isinstance(args[0], list):
                self.send_command(f'position fen {kwargs.get("fen", chess.STARTING_FEN)} moves ' + ' '.join(map(str, args[0])))
        else:
            raise ValueError('Invalid arguments')

    def load_game(self, board: chess.Board, fen: str = chess.STARTING_FEN) -> None:
        """
        Load a game into the engine, based on the board's move stack and a FEN string
        """
        self.set_position(board.move_stack, fen=fen)


    def set_search_options(self, options: SearchOptions) -> None:
        """
        Set the search options for the engine
        """
        self.search_options = options
    
    
    def get_evaluation(self) -> typing.Generator[Evaluation, None, None]:
        """
        Run search and get the evaluation of the current position, in real-time,
        This is a generator that yields Evaluation objects
        """
        self.send_command(f'go {str(self.search_options)}')
        prev_evaluation = Evaluation()

        while True:
            line = self._try_read_line()

            # If the engine didn't output anything, yield the previous evaluation
            if line is None:
                yield prev_evaluation
                continue

            if line.startswith('bestmove'):
                break
            
            prev_evaluation._from_string(line)
            yield prev_evaluation

    def get_bestmove(self) -> chess.Move | None:
        """
        Search for the best move in the current position,
        waits for the engine to finish searching

        :param options: Search options
        :return: Best move found by the engine as a chess.Move object
        """
        self.send_command(f"go {str(self.search_options)}")
        return self._read_until_bestmove()
    

    def isready(self) -> bool:
        """
        Check if the engine is ready
        """
        self.send_command('isready')
        while self._read_line() != 'readyok':
            pass
        
        return True
    

    def stop(self) -> None:
        """
        Stop the engine from searching
        """
        self.send_command('stop')
    

    def quit(self) -> None:
        """
        Quit the engine
        """
        self.send_command('quit')
        self.process.wait()
        self.process.kill()
        self.killed = True
    