A simple chess bot that runs on Lichess.

## Engine

This engine uses alpha-beta pruning to explore the search tree, along with a bunch of heuristics to further prune the search ([more details](docs/engine.md)).

The Lichess bot can be found [here](https://lichess.org/@/penguin_bot). It accepts challenges with a time control of rapid or faster.

## Building

Vcpkg is required for package management. Set `VCPKG_ROOT` to its installation folder.

```bash
# Release builds
cmake --preset=release
cmake --build --preset=release

# Debug builds
cmake --preset=debug
cmake --build --preset=debug
```

This project uses C++23 features such as the `<expected>` library. It has been tested to compile on gcc version >= 13. If the build commands fail with your default compiler, please try with a newer compiler version. For example, to use gcc-14, run: `CC=gcc-14 CXX=g++-14 cmake --preset=release`.


## Running

After creating a release build, configure your bot using a `.env` file with the following information:

```bash
LICHESS_TOKEN=xxx            # API access token of your bot with BOT permissions
LICHESS_BOT_NAME=yyy         # Username of your bot
ISSUE_CHALLENGES=TRUE/FALSE  # Whether the bot should periodically challenge other bots
```

Then run the bot:

```bash
./build/release/bin/lichess_bot
```

## Testing

```bash
./build/debug/test/chess/chess_tests          # To test the chess library
./build/debug/tests/chess_engine_tests        # To test the chess engine
./build/release/test/benchmark/benchmarks     # To run performance benchmarks
```

## Strength Testing

TODO: Instructions for how to test this engine against other engines / older versions of itself.

Development is done in the `dev` branch. Once a new version (typically with better strength) is ready, it will be merged into `main`, and tagged with `vx.y.z`.

To build older versions (tagged with some `vx.y.z`) of the engine_cli binary, use the script `./scripts/build_engine_version.sh` (run it for usage info). Remember to set the environment variables `CC` and `CXX` if you need to use a different compiler.