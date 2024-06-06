#include "chess_game.h"

namespace ChessGame {

const Move &MoveAction::get_move() const { return move; }

bool MoveAction::operator==(const MoveAction &other) const { return move == other.move; }

bool BoardState::is_terminal() const { return !board.has_moves(); }

std::vector<std::pair<BoardState, MoveAction>> BoardState::get_transitions() const {
  std::vector<std::pair<BoardState, MoveAction>> transitions;
  auto moves = board.generate_moves();
  transitions.reserve(moves.size());
  for (size_t i{0}; i < moves.size(); i++) {
    BoardState new_state{board.apply_move(moves[i]), me};
    MoveAction action{moves[i]};
    transitions.emplace_back(new_state, action);
  }
  return transitions;
}

std::optional<MoveAction> BoardState::get_random_action(std::mt19937 &rng) const {
  auto moves{board.generate_moves()};
  if (moves.empty()) return std::nullopt;
  std::uniform_int_distribution<size_t> dist{0, moves.size() - 1};
  size_t index{dist(rng)};
  return moves[index];
}

BoardState BoardState::apply_action(const MoveAction &action) const {
  return BoardState{board.apply_move(action.get_move()), me};
}

float BoardState::get_score() const {
  if (!board.is_in_check()) {
    return 0.5;
  }
  return board.get_color() == me ? 0 : 1;
}

}  // namespace ChessGame
