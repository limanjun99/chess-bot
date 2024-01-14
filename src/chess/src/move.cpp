#include "move.h"

std::string Move::to_uci() const {
  std::string uci{bit::to_algebraic(from) + bit::to_algebraic(to)};
  if (promotion_piece != Piece::None) uci += piece::to_char(promotion_piece);
  return uci;
}

bool Move::operator==(const Move& move) const {
  return piece == move.piece && from == move.from && to == move.to && promotion_piece == move.promotion_piece;
}
