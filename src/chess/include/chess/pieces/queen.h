#include "../bitboard.h"
#include "base_piece.h"

class Queen : public BasePiece<Queen> {
public:
  // Returns a bitboard of squares attacked by a queen on `square`, with pieces on `occupancy`.
  static u64 attacks(u64 square, u64 occupancy);

private:
  static constexpr char character{'q'};
  static constexpr bool slider{true};
  static constexpr PieceVariant variant{PieceVariant::Queen};
  friend class BasePiece<Queen>;
};