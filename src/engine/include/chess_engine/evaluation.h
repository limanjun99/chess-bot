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
                    8,  12, 16, 16, 16, 16, 12, 8,   //
                    12, 16, 20, 24, 24, 20, 16, 12,  //
                    16, 20, 32, 32, 32, 32, 20, 16,  //
                    16, 24, 32, 32, 32, 32, 24, 16,  //
                    16, 24, 32, 32, 32, 32, 24, 16,  //
                    16, 20, 32, 32, 32, 32, 20, 16,  //
                    12, 16, 20, 24, 24, 20, 16, 12,  //
                    8,  12, 16, 16, 16, 16, 12, 8,
                }},
                {{
                    // Pawn.
                    0,  0,  0,  0,  0,  0,  0,  0,   //
                    25, 25, 25, 25, 25, 25, 25, 25,  //
                    20, 20, 20, 20, 20, 20, 20, 20,  //
                    15, 15, 15, 15, 15, 15, 15, 15,  //
                    10, 10, 10, 10, 10, 10, 10, 10,  //
                    5,  5,  5,  5,  5,  5,  5,  5,   //
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