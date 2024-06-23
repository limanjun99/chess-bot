#include "move.h"

#include <cctype>

#include "piece.h"

using namespace chess;

std::string Move::to_uci() const {
  std::string uci{get_from().to_algebraic() + get_to().to_algebraic()};
  if (is_promotion()) uci += piece_variant::to_char(get_promotion_piece());
  return uci;
}

std::string Move::to_algebraic() const {
  char piece_char{static_cast<char>(std::toupper(piece_variant::to_char(get_piece())))};
  std::string uci{piece_char + get_from().to_algebraic() + get_to().to_algebraic()};
  if (is_promotion()) uci += piece_variant::to_char(get_promotion_piece());
  return uci;
}
