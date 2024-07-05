A simple chess bot that runs on Lichess.

## Engine

This engine uses alpha-beta pruning to explore the search tree, along with a bunch of heuristics to further prune the search ([more details](docs/engine.md)).

The Lichess bot can be found [here](https://lichess.org/@/penguin_bot). It accepts challenges with a time control of rapid or faster.

## Building

Vcpkg is required for package management. Set `VCPKG_ROOT` to its installation folder.

```
# Release builds
cmake --preset=release
cmake --build --preset=release

# Debug builds
cmake --preset=debug
cmake --build --preset=debug
```

## Running

After creating a release build, configure your bot using a `.env` file with the following information:

```bash
LICHESS_TOKEN=xxx            # API access token of your bot with BOT permissions
LICHESS_BOT_NAME=yyy         # Username of your bot
ISSUE_CHALLENGES=TRUE/FALSE  # Whether the bot should periodically challenge other bots
```

Then run the bot:

```bash
./build/release/src/lichess/lichess_bot
```

## Testing

```bash
./build/debug/test/chess/chess_tests          # To test the chess library
./build/debug/test/engine/chess_engine_tests  # To test the chess engine
./build/release/test/benchmark/benchmarks     # To run performance benchmarks
```
