# Viking
Viking is a **UCI compatible** chess engine written in **C++ 20**. Viking's **playing strength** is not really known at this point and more robust testing is required to confirm. I suspect it could be around **2300-2400 ELO** since it obliterates
simplistic chess engines.

## Play vs Viking
If you are interested in playing versus Viking chess engine, you can do so on **Lichess @ [TheVikingEngine](https://lichess.org/@/TheVikingEngine)**.

## About the engine
Viking is programmed and fine-tuned in a way to play **aggressive** interesting chess. It will **avoid draws** at all cost, even if that results in a loss. Playing in this fashion makes the games **very entertaining and fun to watch**.

## Board representation
Viking uses the **bitboard approach** for the board representation. Internally it keeps track of **12 bitboards** (one per piece type for each color), as well as **3 bitboard** for occupied squares (white, black and both). The internal board class
also has flags for **side-to-move**, **castling rights**, **en passant**, **fifty move rule** counter etc. Board class also makes it convinient to store past move **history**, as well as heuristics such as **killer** and **history moves**. **Zobrist hashkey** is used 
to differenciate each unique chess position and aids in Viking's **transposition table** implementation. The choice for this board representation approach is clear: ***speed and performance***.

## Move generator
Viking is using **precomputed bitboard attack tables** for generating **pawn**, **knight** and **king** moves. For slider pieces (**bishops**, **rooks** and **queens**), Viking uses **Hyperbola Quintessence** move generation. Precomputed attack tables are already the
optimal way of generating certain piece type moves, wherease **Hyperbola Quintessence** could potentially be replaced with **Magic bitboards** in the future for even more performance. That being said, Viking's move generator is currently
producing **PERFT** speeds upwards of ***18M nodes/second*** on regular consumer-grade hardware.

## Search
Viking employs traditional **negamax alpha-beta pruning** search algorithm with **many optimizations** added on top. These include **iterative deepening**, **null-move pruning**, **principal variation search**, **transposition table**, **late move reductions** etc.

## Move ordering
**Negamax**: $O(b^d)$ &nbsp;&nbsp;&nbsp; **Alpha-beta (best case)**: $O(b^{d/2})$

In order to make **alpha-beta pruning** as **efficient** as possible, **ordering of moves** is paramount.

- **Transposition table** - If the move is already found in our transposition table, we can just use it.
- **Principal variation** - Best move from the previous search depth is likely to be a good move in the deeper search.
- **MVVLVA** - Most valuable victim, least valuable attacker. Viking orders captures by relative piece value, where bigger score is given to the move where a low value piece (eg. pawn) captures a higher value piece (eg. bishop).
- **Killer moves** - Heuristic used for ordering of quiet moves which caused a beta-cutoff in a sibling node.
- **History moves** - Heuristic used for ordering of quiet moves which caused an increase in alpha in the sibling node.
- **Possible attacks** - Heuristic used for ordering of quiet moves which, if played, would attack an opponents piece. It is similar in nature to MVVLVA, but used for quiet moves instead of capture moves.

Combining all of these **move ordering techniques** causes Viking to pick the **optimal move** within the **first few moves** around **85%** of the time meaning that only about **15%** of the time, it must go through more than a **couple of moves** to obtain a **cuttoff**.

## Evaluation
Viking is using **traditional tapered evaluation** (**not neural networks**) for statically evaluating it's **positions**.

#### General
- **Material** - Value of each piece on the board weighted by opening/endgame phases.
- **Positional** - Uses piece-square tables to evaluate position of each piece weighted by opening/endgame phases.
- **Space control** - Bonus points for controlling space on the opponent's side of the board.
- **Center control** - Most pieces are strongest in the center, so bonus is given for strong central presence.
- **Tempo** - Bonus for having the right to make a move, it promotes attacking chess.

#### King safety
- **Files** - Penalty for having open or half-open files in front of the castled king.
- **King ring** - Penalty for having too many opponent's pieces near your king.

#### Pawns
- **Doubled pawns** - Penalty is given for doubled or trippled pawns because they can be easily captured.
- **Isolated pawns** - Penalty is give for having pawns without other pawns on either side because they need stronger pieces to defend them.
- **Passed pawns** - Bonus is given for having a passed pawn because it can often promote in the endgame and is difficult to guard.
- **Undefended pawns** - Penalty for having pawns which are not currently defended by other pawns.

#### Knights
- **Mobility** - Bonus for each possible square the knight can jump to.
- **Closed position** - Bonus for having a knight in closed positions because they are more useful than bishops.
- **Undefended knights** - Penalty for having knights which are not currently defended by pawns (outposts).

#### Bishops
- **Mobility** - Bonus for each possible move the bishop can slide to.
- **Bishop pair** - Bonus for having two bishops because they work better in tandem.
- **Blocked bishop** - Penalty for having a bishop blocked by pawns (tall pawn).
- **Color complex** - Bonus for having bishops which can infiltrate gaps in opponent's defenses

#### Rooks
- **Mobility** - Bonus for each possible move the rook can slide to.
- **Files** - Bonus for having rooks on open/half-open files.

#### Queens
- **Mobility** - Bonus for each possible move the queen can slide to.

## UCI Protocol
Viking uses **UCI protocol** which is the most commonly used protocol for **communication** between chess engines and GUIs or other interfaces. This allows Viking to be **connected** to popular software like **Arena** or **Nibbler**.

## I am waiting for you :) @TusKabina
