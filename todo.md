## Move generation
- [x] Generate all possible moves for a given board state:
  - [x] Pawn moves:
    - [x] Valid pinned pawn moves -> when a pawn is pinned to the king, and tries to enpassant capture
    - [x] Promotion moves (-.-)
  - [x] Overall:
    - [x] When king is in check -> the first check works fine, but every subsequent check the pieces doesn't care about the king's situation
    - [x] king check detection -> valid move generation for king when in check by sliding pieces
    - [x] Perft test 
    - [x] Game over detection
    - [x] 50 move rule detection
    - [x] Threefold repetition detection
- [x] UCI basic commands
  - [x] `uci`
  - [x] `isready`
  - [x] `ucinewgame`
  - [x] `position` (fen, startpos)
  - [x] `go` (perft, search)
  - [x] `stop`
  - [x] `quit`
- [x] Search
  - [x] Quiescence search
  - [x] Move ordering
    - [x] History heuristic (for quiet moves)
    - [x] For captures: MVV/LVA
  - [x] Iterative deepening
  - [x] Time management
  - [x] Engine badly detects treefold repetition (user may force a draw in a winning position for the engine)
  - [ ] Engine can't win winning endgames:
    - [ ] KQ vs k
    - [ ] KR vs k
  - [x] BUG: King sometimes can be captured:
    - 6k1/5pp1/1Q2b2p/4P3/7P/8/3r2PK/3q4 w - - 1 34
    - r4rk1/pppb1p2/3bq2p/3NN2Q/2B3p1/8/PP1R2PP/4R2K b - - 0 23
- [x] Evaluation
  - [x] Pawn structure (phalanx) 

Code related:
- [x] refactor `search` function, it's a mess
- [x] refactor ALL of the code

- [ ] write own hash table, since std::unordered_map is hasing the board hash again
- [ ] fix search with alpha-beta:
  - [ ] zw search not working (no asp window)
  - [ ] alpha beta with asp window + normal search
  - search alone seems fine, should benchmark eval:
    - v0 vs v0
      - CEngine_v0: 30
      - CEngine_v0: 27
      - Draw: 43
    - v0 vs v3 (base model)
      - CEngine_v0: 25
      - CEngine_v3: 28
      - Draw: 47
    - v0 vs v3 (eval upgrade)
      - CEngine_v0: 33
      - CEngine_v3: 31
      - Draw: 36


### May 3, 4, 5 2025
- [x] new console ui option as an argument (--ui), play against the computer
- [x] On startup with the --ui option, set the fen position (same as position arg), side of the player, and engine limits (without infinite)
- [x] Add arguments to the cmd line for --ui, --ui-fen, --ui-side, --ui-limits
- [x] Way more interesting ui, with ansi escape codes
- [x] Make a better arg parser, with new types: boolean, int, string, list
- [ ] Analysis mode
- [ ] Fix the Game Summary in single player mode (wait for the user to press enter or smth)

### Bugs
- [ ] Detected a bug in the search, there is a chance that the engine will not find the best move, and make a clear blunder (losing a queen or a piece for no reason)


# New matches

## v0 vs v0
- CEngine_v0: 135
- CEngine_v0: 142
- Draw: 219

## v3.1 vs v0
- CEngine_v31: 142
- CEngine_v0: 116
- Draw: 238

## v3.2 vs v0
- CEngine_v32: 160
- CEngine_v0: 115
- Draw: 221

v3.3 vs v0 (eval king safety) **(FAILED)**:
- CEngine_v33: 151
- CEngine_v0: 124
- Draw: 222

v3.3 vs v0 (mahattan heuristic) **(FAILED)**:
- CEngine_v33: 156
- CEngine_v0: 139
- Draw: 200

```cpp
// Step 5: Evaluate the king position in the winning endgame
// if the endgame factor is high and one side has a significant advantage
// then the winning side's king should be more active and 
// try to get closer to the enemy king
if (
    (material >= piece_values[Piece::Rook - 1]) 
    && ((board.rooks(is_white) | board.queens(is_white)) != 0 
        && (board.rooks(is_enemy) | board.queens(is_enemy)) == 0))
{
    Square king       = bit_scan_forward(board.m_bitboards[is_white][Piece::King - 1]);
    Square enemy_king = bit_scan_forward(board.m_bitboards[is_enemy][Piece::King - 1]);

    // If it's winning side, apply bonus for being close to the enemy king
    eval += 6 * (14 - manhattan_distance[king][enemy_king]);
}
```

## v3.3 vs v0 (aspiration window)
- CEngine_v33: 177
- CEngine_v0: 105
- Draw: 211

```cpp
// In the seach loop...

// Aspiration window
while(true)
{
    eval = search<Root>(m_board, alpha, beta, m_depth);

    if (eval <= alpha)
    {
        alpha = std::max(alpha - delta, MIN);
    }
    else if (eval >= beta)
    {
        beta = std::min(beta + delta, MAX);
    }
    else
    {
        break;
    }

    delta += delta / 2;
}

// eval = search<Root>(m_board, alpha, beta, m_depth);
if (abs(eval) >= MATE_THRESHOLD)
    m_result.pv       = get_pv(MAX_PLY);
else
    m_result.pv       = get_pv(m_depth);

// Save the pv
m_root_pv         = m_result.pv;
m_result.bestmove = m_result.pv.size() > 0 ? m_result.pv[0] : m_bestmove;

// Update alpha beta
alpha  = eval - delta;
beta   = eval + delta;

// rest of the code...

```

## v3.4 vs v0 (3.3 + killer heuristic)
- CEngine_v34: 190
- CEngine_v0: 100
- Draw: 204

(2nd match)
- CEngine_v35: 183
- CEngine_v0: 101
- Draw: 211

## 3.5 vs v0 (3.4 + zw search)
- CEngine_v35: 187
- CEngine_v0: 91
- Draw: 217

## 3.5 vs v0 (3.4 + zw search + move ordering upgrade)
- CEngine_v35: 248
- CEngine_v0: 55
- Draw: 194