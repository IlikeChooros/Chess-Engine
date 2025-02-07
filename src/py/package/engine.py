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
    killed: bool                  = False
    search_options: SearchOptions = SearchOptions()
    name: str                     = 'Engine'
    should_print: bool            = False
    is_running: bool              = False
    last_line: str                = ''

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

        self.send_command('uci')
        self._read_until('uciok')

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
        self.last_line = out

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
    
    def _read_until(self, until: str) -> str:
        """
        Read the output of the engine until a certain string is found
        """
        while True:
            line = self._read_line()
            if line.startswith(until):
                self.last_line = line
                return line
    
    def _read_until_bestmove(self, line: str | None = None) -> chess.Move | None:
        """
        Read the output of the engine until the `bestmove` is found.

        This is used to get the best move after a search command

        :return: string containing the bestmove output, ex: 'bestmove e2e4'
        """
        if line is None:
            line = self._read_until('bestmove')
        strmove = line.split()[1]
        move: chess.Move | None = None
        if strmove != '(none)':
            move = chess.Move.from_uci(strmove)
        self.is_running = False
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
        
        if command.startswith('go'):
            self.is_running = True
        
        if command.startswith(('stop', 'quit')):
            self.is_running = False

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
    
    @typing.overload
    def set_position(self, fen: str = chess.STARTING_FEN) -> None:
        """
        Set the position of the engine, based on a FEN string
        """
        ...

    def set_position(self, *args, **kwargs) -> None:
        # Implement the overloads
        if len(args) == 1:
            if isinstance(args[0], chess.Board):
                self.send_command('position fen ' + args[0].fen())

            elif isinstance(args[0], list):
                if len(args[0]) == 0:
                    self.send_command(f'position fen {kwargs.get("fen", chess.STARTING_FEN)}')
                else:
                    self.send_command(f'position fen {kwargs.get("fen", chess.STARTING_FEN)} moves ' + ' '.join(map(str, args[0])))
            
            elif isinstance(args[0], str):
                self.send_command('position fen ' + args[0])
        
        elif len(args) == 0:
            self.send_command('position fen ' + kwargs.get('fen', chess.STARTING_FEN))
        else:
            raise ValueError('Invalid arguments')

    def load_game(self, board: chess.Board, fen: str = chess.STARTING_FEN) -> None:
        """
        Load a game into the engine, based on the board's move stack and a FEN string
        """
        moves: str = ''

        if len(board.move_stack) != 0:
            moves = 'moves ' + ' '.join(map(str, board.move_stack))
        self.send_command('position fen %s %s' % (fen, moves))


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
                self.is_running = False
                break
            
            prev_evaluation._from_string(line)
            yield prev_evaluation

    def go(self, options: typing.Optional[SearchOptions] = None) -> None:
        """
        Run the search command with the specified options
        """
        if options is not None:
            self.set_search_options(options)
        
        self.send_command(f'go {str(self.search_options)}')

    def get_bestmove(self) -> chess.Move | None:
        """
        Get the best move, after the 'go' command
        :return: Best move found by the engine as a chess.Move object or None
        """
        return self._read_until_bestmove()
    

    def try_bestmove(self) -> chess.Move | None:
        """
        Try to get the best move, if the engine is not running, return None
        """        
        line = self._try_read_line()        
        if line is not None and line.startswith('bestmove'):
            return self._read_until_bestmove(line)

        return None

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
    