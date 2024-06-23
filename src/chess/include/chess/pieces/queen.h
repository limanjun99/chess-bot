#include "../bitboard.h"
#include "base_piece.h"

namespace chess {

class Queen : public BasePiece<Queen> {
public:
  // Returns a bitboard of squares attacked by a queen on `square`, with pieces on `occupancy`.
  static Bitboard attacks(Bitboard square, Bitboard occupancy);

private:
  static constexpr char character{'q'};
  static constexpr bool slider{true};
  static constexpr PieceVariant variant{PieceVariant::Queen};
  friend class BasePiece<Queen>;
};

}  // namespace chess