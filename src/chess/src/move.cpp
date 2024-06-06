#include "move.h"

#include <cctype>

#include "board.h"
#include "piece.h"

std::string Move::to_uci() const {
  std::string uci{bit::to_algebraic(from) + bit::to_algebraic(to)};
  if (promotion_piece != PieceVariant::None) uci += piece_variant::to_char(promotion_piece);
  return uci;
}

std::string Move::to_algebraic(const Board& board) const {
  PieceVariant piece{board.cur_player().piece_at(from)};
  char piece_char{static_cast<char>(std::toupper(piece_variant::to_char(piece)))};
  std::string uci{piece_char + bit::to_algebraic(from) + bit::to_algebraic(to)};
  if (promotion_piece != PieceVariant::None) uci += piece_variant::to_char(promotion_piece);
  return uci;
}
