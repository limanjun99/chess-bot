#include "../bitboard.h"
#include "base_piece.h"

class Bishop : public BasePiece<Bishop> {
public:
  // Returns a bitboard of squares attacked by a bishop on `square`, with pieces on `occupancy`.
  static u64 attacks(u64 square, u64 occupancy);

private:
  static constexpr char character{'b'};
  static constexpr bool slider{true};
  static constexpr PieceVariant variant{PieceVariant::Bishop};
  friend class BasePiece<Bishop>;
};