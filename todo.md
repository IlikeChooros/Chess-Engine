## Move generation
- [ ] Generate all possible moves for a given board state:
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
- [ ] Search
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
- [ ] Evaluation
  - [ ] Pawn structure (phalanx) 

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


# New matches

v3.1 vs v0:
- CEngine_v31: 159
- CEngine_v0: 107
- Draw: 232

(manhattan dist) vs v0:
- CEngine_v31: 142
- CEngine_v0: 116
- Draw: 238