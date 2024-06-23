#include "move.h"

#include <cctype>

#include "board.h"
#include "piece.h"

using namespace chess;

std::string Move::to_uci() const {
  std::string uci{from.to_algebraic() + to.to_algebraic()};
  if (promotion_piece != PieceVariant::None) uci += piece_variant::to_char(promotion_piece);
  return uci;
}

std::string Move::to_algebraic(const Board& board) const {
  PieceVariant piece{board.cur_player().piece_at(from)};
  char piece_char{static_cast<char>(std::toupper(piece_variant::to_char(piece)))};
  std::string uci{piece_char + from.to_algebraic() + to.to_algebraic()};
  if (promotion_piece != PieceVariant::None) uci += piece_variant::to_char(promotion_piece);
  return uci;
}
