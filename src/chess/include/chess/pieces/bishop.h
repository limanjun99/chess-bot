#include "../bitboard.h"
#include "base_piece.h"

namespace chess {

class Bishop : public BasePiece<Bishop> {
public:
  // Returns a bitboard of squares attacked by a bishop on `square`, with pieces on `occupancy`.
  static Bitboard attacks(Bitboard square, Bitboard occupancy);

private:
  static constexpr char character{'b'};
  static constexpr bool slider{true};
  static constexpr PieceType type{PieceType::Bishop};
  friend class BasePiece<Bishop>;
};

}  // namespace chess