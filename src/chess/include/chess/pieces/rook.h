#include "../bitboard.h"
#include "base_piece.h"

namespace chess {

class Rook : public BasePiece<Rook> {
public:
  // Returns a bitboard of squares attacked by a rook on `square`, with pieces on `occupancy`.
  static Bitboard attacks(Bitboard square, Bitboard occupancy);

private:
  static constexpr char character{'r'};
  static constexpr bool slider{true};
  static constexpr PieceVariant variant{PieceVariant::Rook};
  friend class BasePiece<Rook>;
};

}  // namespace chess