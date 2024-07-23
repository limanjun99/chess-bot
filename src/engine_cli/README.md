# UCI Interface

This application provides a Command Line Interface over our chess engine. It currently supports a small subset of the UCI (Universal Chess Interface) protocol, with plans to include more features eventually.

- [x] `position` and `stop` commands.
- [x] Partial support for `go` command.
  - [x] If `movetime` is provided, then the search will complete within that time.
  - [x] If `wtime, btime, winc, binc` are provided, then the engine will spend an appropriate amount of time searching.
- [ ] `isready` currently has a wrong implementation that blocks all incoming commands (e.g. `go`, `isready`, `stop` will hang as the `stop` command is never read).
- [ ] `quit` command doesn't work if in the middle of a search.
- [ ] Debug information (when debug mode is enabled through `debug on`, our engine should return information through `info`).

The purpose of this interface is to eventually allow for easier strength testing of modifications to the engine. Currently, this engine is able to run in cutechess with time controls of fixed time per move.

## Running

```bash
./build/release/bin/engine_cli
```

## Unit Testing

```bash
./build/debug/tests/engine_cli_tests
```
