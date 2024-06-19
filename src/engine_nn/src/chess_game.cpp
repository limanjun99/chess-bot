#include "chess_game.h"

#include "model.h"

namespace ChessGame {

const Move &MoveAction::get_move() const { return move; }

bool MoveAction::operator==(const MoveAction &other) const { return move == other.move; }

float MoveAction::get_density(torch::Tensor policy) const {
  const auto policy_view{policy.accessor<float, 3>()};

  const int from_index{bit::to_index(move.get_from())};
  const int from_y{from_index / 8};
  const int from_x{from_index % 8};

  // Planes 64~67 are for promotions.
  if (move.is_promotion()) {
    const PieceVariant promotion_piece{move.get_promotion_piece()};
    if (promotion_piece == PieceVariant::Bishop) return policy_view[64][from_y][from_x];
    if (promotion_piece == PieceVariant::Knight) return policy_view[65][from_y][from_x];
    if (promotion_piece == PieceVariant::Rook) return policy_view[66][from_y][from_x];
    /*if (promotion_piece == PieceVariant::Queen)*/ return policy_view[67][from_y][from_x];
  }

  const int to_index{bit::to_index(move.get_to())};
  return policy_view[to_index][from_y][from_x];
}

void MoveAction::set_density(torch::Tensor policy, float density) const {
  auto policy_view{policy.accessor<float, 3>()};

  const int from_index{bit::to_index(move.get_from())};
  const int from_y{from_index / 8};
  const int from_x{from_index % 8};

  // Planes 64~67 are for promotions.
  if (move.is_promotion()) {
    const PieceVariant promotion_piece{move.get_promotion_piece()};
    if (promotion_piece == PieceVariant::Bishop) policy_view[64][from_y][from_x] = density;
    else if (promotion_piece == PieceVariant::Knight) policy_view[65][from_y][from_x] = density;
    else if (promotion_piece == PieceVariant::Rook) policy_view[66][from_y][from_x] = density;
    else /*if (promotion_piece == PieceVariant::Queen */ policy_view[67][from_y][from_x] = density;
    return;
  }

  const int to_index{bit::to_index(move.get_to())};
  policy_view[to_index][from_y][from_x] = density;
}

BoardState BoardState::initial() { return BoardState{Board::initial()}; }

bool BoardState::is_terminal() const { return board.is_game_over(); }

const Board &BoardState::get_board() const { return board; }

std::vector<std::pair<BoardState, MoveAction>> BoardState::get_transitions() const {
  std::vector<std::pair<BoardState, MoveAction>> transitions;
  auto moves = board.generate_moves();
  transitions.reserve(moves.size());
  for (size_t i{0}; i < moves.size(); i++) {
    BoardState new_state{board.apply_move(moves[i])};
    MoveAction action{moves[i]};
    transitions.emplace_back(new_state, action);
  }
  return transitions;
}

BoardState BoardState::apply_action(const MoveAction &action) const {
  return BoardState{board.apply_move(action.get_move())};
}

std::optional<float> BoardState::get_player_score() const {
  if (const auto score{board.get_player_score()}) {
    // Normalize score from [-1, 1] to [0, 1].
    return (*score + 1.0) / 2.0;
  }
  return std::nullopt;
}

torch::Tensor BoardState::to_tensor() const {
  torch::Tensor x{torch::zeros({model::input_channels, model::input_height, model::input_width})};
  auto x_view{x.accessor<float, 3>()};

  // Planes 0~5 are for white pieces.
  // Planes 6~11 are for black pieces.
  for (int plane{0}; plane < 12; plane++) {
    const PieceVariant piece{static_cast<PieceVariant>(plane % 6)};
    const Player &player{plane < 6 ? board.get_white() : board.get_black()};
    const u64 piece_bitboard{player[piece]};
    BITBOARD_ITERATE(piece_bitboard, square) {
      const int index{bit::to_index(square)};
      const int y{index / 8};
      const int x{index % 8};
      x_view[plane][y][x] = 1.0;
    }
  }

  // Plane 12 is for en passant.
  const u64 en_passant_square{board.get_en_passant()};
  if (en_passant_square) {
    const int index{bit::to_index(en_passant_square)};
    const int y{index / 8};
    const int x{index % 8};
    x_view[12][y][x] = 1.0;
  }

  // Plane 13 is for castling.
  if (board.get_white().can_castle_kingside()) x_view[13][0][6] = 1.0;
  if (board.get_white().can_castle_queenside()) x_view[13][0][2] = 1.0;
  if (board.get_black().can_castle_kingside()) x_view[13][7][6] = 1.0;
  if (board.get_black().can_castle_queenside()) x_view[13][7][2] = 1.0;

  // Plane 14 is for the current player's color.
  if (board.is_white_to_move()) {
    for (int y{0}; y < 8; y++) {
      for (int x{0}; x < 8; x++) {
        x_view[14][y][x] = 1.0;
      }
    }
  }

  return x;
}

}  // namespace ChessGame
