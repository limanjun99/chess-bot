#pragma once

#include <array>

#include "chess/bitboard.h"
#include "chess/piece.h"

// Piece Square Tables
namespace pst {

// PST values indexed by [color][piece][position].
constexpr std::array<std::array<std::array<int, 64>, 6>, 2> values = ([]() {
  std::array<std::array<std::array<int, 64>, 6>, 2> values;
  // These values are written from white's perspective of the chessboard for easier reading, but are flipped in terms of
  // y-coordinate, hence it is assigned to black instead.
  values[0] = {{{{
                    // Bishop.
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,
                }},
                {{
                    // King.
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,
                }},
                {{
                    // Knight.
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,
                }},
                {{
                    // Pawn.
                    0,  0,  0,  0,  0,  0,  0,  0,   //
                    20, 20, 20, 20, 20, 20, 20, 20,  //
                    15, 15, 15, 16, 16, 15, 15, 15,  //
                    10, 10, 11, 12, 12, 11, 10, 10,  //
                    4,  5,  6,  8,  8,  6,  5,  4,   //
                    2,  2,  3,  4,  4,  3,  2,  2,   //
                    0,  0,  0,  0,  0,  0,  0,  0,   //
                    0,  0,  0,  0,  0,  0,  0,  0,
                }},
                {{
                    // Queen.
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,
                }},
                {{
                    // Rook.
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 0, 0, 0, 0, 0,  //
                    0, 0, 0, 1, 0, 1, 0, 0,
                }}}};
  for (int i = 0; i < 6; i++) {
    for (int j = 0; j < 64; j++) {
      const int y = 7 - j / 8;
      const int x = j % 8;
      values[1][i][j] = values[0][i][(y * 8 + x)];
    }
  }
  return values;
})();

// Returns the PST value of a piece at the given position.
int value_of(Piece piece, u64 at, bool is_white_turn);
};  // namespace pst

inline int pst::value_of(Piece piece, u64 at, bool is_white_turn) {
  int index = bit::to_index(at);
  return pst::values[is_white_turn][static_cast<int>(piece)][index];
}