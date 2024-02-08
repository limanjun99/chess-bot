#include "move.h"

std::string Move::to_uci() const {
  std::string uci{bit::to_algebraic(from) + bit::to_algebraic(to)};
  if (promotion_piece != PieceVariant::None) uci += piece_variant::to_char(promotion_piece);
  return uci;
}
