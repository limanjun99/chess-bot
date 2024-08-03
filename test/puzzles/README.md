# Puzzles

This test aims to quantify engine performance by letting the engine solve puzzles and measuring certain statistics.

## Dataset

The file `/dataset/puzzles/puzzles.csv` contains a sample of puzzles taken from [Lichess's open database](https://database.lichess.org/#puzzles).
Each line in the CSV file has the format `PuzzleId,FEN,SolutionMove,Rating`.

| Field        | Description                                          |
| ------------ | ---------------------------------------------------- |
| PuzzleID     | A string that uniquely identifies this puzzle.       |
| FEN          | FEN of the puzzle position.                          |
| SolutionMove | A UCI string of the correct move to play.            |
| Rating       | Puzzle difficulty for humans.                        |

There is a corresponding `/dataset/puzzles/statistics.csv` file containing statistics of engine performance when solving the above puzzles.
Each line in the statistics file has the format `PuzzleId,Solved,NormalNodeCount,QuiescenceNodeCount,SearchDepth,TimeMS`.
Note that if `Solved=0`, that means the engine timed out while searching, and the statistics are of that search.

| Field               | Description                                                     |
| ------------------- | --------------------------------------------------------------- |
| PuzzleID            | A string that uniquely identifies this puzzle.                  |
| Solved              | 1 if the engine solved the puzzle, else 0.                      |
| NormalNodeCount     | Number of non-quiescence nodes searched.                        |
| QuiescenceNodeCount | Number of quiescence nodes searched.                            |
| SearchDepth         | The depth the engine searched to, to solve the puzzle.          |
| TimeMS              | Time taken (in milliseconds) to reach the above search depth.   |

The initial dataset has been generated with the command `./build/release/tests/puzzles ./dataset/puzzles <path_to_lichess_db_puzzles.csv>`,
where `lichess_db_puzzles.csv` is the downloaded CSV file from Lichess's open database of puzzles.

## Testing

To test the current engine's performance:

```bash
./build/release/tests/puzzle ./dataset/puzzles
```

This will generate a `new_statistics.csv` file in the `/dataset/puzzles` directory, and print the difference between the two statistics to stdout.
To update the statistics, simply replace the original `statistics.csv` file with the new one and commit the change.

The current set of statistics has been generated on a Apple M2 Macbook Air with 8GB RAM.

## Warning

I have no idea if there is even any correlation between puzzle solving performance and real match performance.
SPRT is too expensive to be used for every little commit, so this puzzle testing will have to do for now.

However, a SPRT should still be ran before every version release, to ensure that the changes actually helped match performance.