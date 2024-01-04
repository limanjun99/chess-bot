A simple chess bot that runs on Lichess.

The current engine is a naive bruteforce with extremely low depth. It seems to be able to avoid one-move blunders, and is able to spot hanging pieces, but beyond that it is extremely weak and will only shuffle pieces around.

The Lichess bot will only accept unrated challenges, and only plays against one challenger at any time. Try it out [here](https://lichess.org/@/penguin_bot).

## Building

```bash
cmake -S . -B ./build
cmake --build ./build
```

## Running

Configure your bot using a `.env` file with the following information:

```bash
LICHESS_TOKEN=xxx       # API access token of your bot with BOT permissions
LICHESS_BOT_NAME=yyy    # Username of your bot
```

Then run the bot:

```bash
./build/src/lichess/lichess_bot
```

## Testing

```bash
./build/test/benchmark/benchmarks       # To run performance benchmarks
./build/test/chess/chess_tests          # To test the base chess library
./build/test/engine/chess_engine_tests  # To test the chess engines
```
