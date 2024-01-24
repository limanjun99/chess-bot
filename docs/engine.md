## Engine

Here is a list of the current heuristics implemented by the engine (full credit to the [Chess Programming Wiki](https://www.chessprogramming.org/)).

- **[Null Move Heuristic](https://www.chessprogramming.org/Null_Move_Pruning)**

  We skip our turn, and continue the search at a reduced depth. If the position's evaluation is estimated to be `>= beta`, then it is extremely likely that NOT skipping our turn also gives an evaluation of `>= beta`. Hence, we can simply beta cutoff if that occurs, and save the effort of move generation / further searching at full depth.

- **[Killer Heuristic](https://www.chessprogramming.org/Killer_Heuristic)**

  At each ply, we store a few moves that are "killer" - moves that kill any chances the opponent has and cause a beta cutoff. These moves should be tried first in when we are at the same depth in hopes of beta cutoff, as it is likely that the move is good in many of the positions at that depth.

- **[Transposition Table](https://www.chessprogramming.org/Transposition_Table)**

  The transposition table is a hash map that stores information about positions we have encountered before. Currently, it stores the type of node, evaluation, and a hash move (the best move). If the same position has been analyzed to a sufficient depth before, we can simply try to reuse the results from that analysis. Otherwise, we can still try to search the hash move first, as that is the most likely branch to cause a beta cutoff.

- **[History Heuristic](https://www.chessprogramming.org/Relative_History_Heuristic)**

  During our search, we keep track of how often we see each move, as well as how often they cause a beta cutoff. This tells us how likely a move is to cause a beta cutoff, so we can get a better move ordering.

- **[Late Move Reductions](https://www.chessprogramming.org/Late_Move_Reductions)**

  With all the other heuristics, we can assume that our move ordering is fairly good, which means that the later moves are likely bad (not good enough to raise alpha). Hence we search those at a reduced depth, but if they do manage to raise alpha, then they are promising enough and we re-search them with full depth to get an accurate evaluation.

- **[Futility Pruning](https://www.chessprogramming.org/Futility_Pruning)**

  When we are at frontier nodes (with 1 depth left), we estimate whether each move's value + some safety margin will bring us above alpha. If not, it likely that quiescence searching it still gives us an evaluation below alpha, so we can just skip it.

## Move Ordering

Moves are searched in the following order:

- Hash move (from the transposition table)
- Captures / promotions (sorted according to [MVV-LVA](https://www.chessprogramming.org/MVV-LVA))
- Killer moves
- All other moves sorted by history heuristic
