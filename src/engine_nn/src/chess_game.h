#pragma once

#include <torch/torch.h>

#include <optional>

#include "chess/board.h"
#include "chess/move.h"

// Contains wrapper classes around the chess library for usage with the engine.
namespace ChessGame {

class MoveAction {
public:
  MoveAction(std::convertible_to<Move> auto &&move) : move{std::forward<decltype(move)>(move)} {}
  const Move &get_move() const;
  bool operator==(const MoveAction &other) const;
  float get_density(torch::Tensor policy) const;
  void set_density(torch::Tensor policy, float density) const;

private:
  Move move;
};

class BoardState {
public:
  BoardState(std::convertible_to<Board> auto &&board) : board{std::forward<decltype(board)>(board)} {}

  static BoardState initial();

  bool is_terminal() const;
  const Board &get_board() const;
  std::vector<std::pair<BoardState, MoveAction>> get_transitions() const;
  BoardState apply_action(const MoveAction &action) const;
  std::optional<float> get_player_score() const;
  torch::Tensor to_tensor() const;

private:
  Board board;
};

}  // namespace ChessGame
