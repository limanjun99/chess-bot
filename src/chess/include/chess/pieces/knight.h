#include "../bitboard.h"
#include "base_piece.h"

class Knight : public BasePiece<Knight> {
public:
  // Returns a bitboard of squares attacked by a knight on `square`.
  static u64 attacks(u64 square);

private:
  static constexpr char character{'n'};
  static constexpr bool slider{false};
  static constexpr PieceVariant variant{PieceVariant::Knight};
  friend class BasePiece<Knight>;
};