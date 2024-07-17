# UCI Interface

This application provides a Command Line Interface over our chess engine. It currently supports a small subset of the UCI (Universal Chess Interface) protocol, with plans to include more features eventually.

- [x] `position` and `stop` commands.
- [x] Partial support for `go` command.
  - [ ] Implement time management when `wtime, btime, winc, binc` are provided.
- [ ] `isready` currently has a wrong implementation that blocks all incoming commands (e.g. `go`, `isready`, `stop` will hang as the `stop` command is never read).
- [ ] `quit` command doesn't work if in the middle of a search.
- [ ] Debug information (when debug mode is enabled through `debug on`, our engine should return information through `info`).
- [ ] Better time management in general (e.g. if `movetime` is provided, how to ensure that we don't exceed it?). This should probably be implemented in the engine itself, and not through this interface.

The purpose of this interface is to eventually allow for easier strength testing of modifications to the engine. Currently, this engine is able to run in cutechess with time controls of fixed time per move.

## Running

```bash
./build/release/src/engine_cli/engine_cli
```

## Unit Testing

```bash
./build/debug/src/engine_cli/engine_cli_tests
```
