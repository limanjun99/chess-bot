#include "move.h"

Move::Move(Piece piece, u64 from, u64 to) : piece{piece}, from{from}, to{to}, promotion_piece{std::nullopt} {}

Move::Move(u64 from, u64 to, Piece promotion_piece)
    : piece{Piece::Pawn}, from{from}, to{to}, promotion_piece{promotion_piece} {}

u64 Move::get_from() { return from; }

u64 Move::get_to() { return to; }

Piece Move::get_piece() { return piece; }

std::optional<Piece> Move::get_promotion_piece() { return promotion_piece; }

std::string Move::to_uci() const {
  std::string uci{bit::to_algebraic(from) + bit::to_algebraic(to)};
  if (promotion_piece) {
    switch (*promotion_piece) {
      case Piece::Bishop:
        uci += 'b';
        break;
      case Piece::Knight:
        uci += 'n';
        break;
      case Piece::Queen:
        uci += 'q';
        break;
      case Piece::Rook:
        uci += 'r';
        break;
      default:
        throw "Unreachable - Move::to_uci";
    }
  }
  return uci;
}
