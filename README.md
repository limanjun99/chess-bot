A simple chess bot that runs on Lichess.

## Engine

The engine uses alpha-beta pruning to explore the search tree, along with a bunch of heuristics to further prune the search ([more details](docs/engine.md)). It is capable of beating up to Fairy Stockfish Level 6 on Lichess (which is not so impressive).

The Lichess bot will only accept unrated challenges, and only plays against one challenger at any time. Try it out [here](https://lichess.org/@/penguin_bot).

## Building

```bash
./scripts/build.sh

./scripts/build.sh -d   # For debug builds:
```

## Running

Configure your bot using a `.env` file with the following information:

```bash
LICHESS_TOKEN=xxx       # API access token of your bot with BOT permissions
LICHESS_BOT_NAME=yyy    # Username of your bot
```

Then run the bot:

```bash
./scripts/run.sh
```

## Testing

```bash
./scripts/test.sh                  # To test the base chess library and chess engine
./build/test/benchmark/benchmarks  # To run performance benchmarks
```
