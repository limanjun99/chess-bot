#pragma once

#include <optional>
#include <random>

#include "chess/board.h"
#include "chess/move.h"

// Contains wrapper classes around the chess library for usage with the engine.
namespace ChessGame {

class MoveAction {
public:
  MoveAction(std::convertible_to<Move> auto &&move) : move{std::forward<decltype(move)>(move)} {}
  const Move &get_move() const;
  bool operator==(const MoveAction &other) const;

private:
  Move move;
};

class BoardState {
public:
  BoardState(std::convertible_to<Board> auto &&board, Color me) : board{std::forward<decltype(board)>(board)}, me{me} {}
  bool is_terminal() const;
  std::vector<std::pair<BoardState, MoveAction>> get_transitions() const;
  std::optional<MoveAction> get_random_action(std::mt19937 &rng) const;
  BoardState apply_action(const MoveAction &action) const;
  float get_score() const;

private:
  Board board;
  // Keep track of which player I am (because Board does not).
  Color me;
};

}  // namespace ChessGame
