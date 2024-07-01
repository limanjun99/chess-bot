#include "pieces/king.h"

#include <array>

using namespace chess;

constexpr std::array<Bitboard, 64> king_lookup = ([]() {
  std::array<Bitboard, 64> king_lookup{};
  for (int index = 0; index < 64; index++) {
    const Bitboard king{Bitboard::from_index(index)};
    king_lookup[index] |= (king & ~Bitboard::rank_8 & ~Bitboard::file_H) << 9;  // Up right.
    king_lookup[index] |= (king & ~Bitboard::rank_8) << 8;                      // Up.
    king_lookup[index] |= (king & ~Bitboard::rank_8 & ~Bitboard::file_A) << 7;  // Up left.
    king_lookup[index] |= (king & ~Bitboard::file_H) << 1;                      // Right.
    king_lookup[index] |= (king & ~Bitboard::file_A) >> 1;                      // Left.
    king_lookup[index] |= (king & ~Bitboard::rank_1 & ~Bitboard::file_H) >> 7;  // Down right.
    king_lookup[index] |= (king & ~Bitboard::rank_1) >> 8;                      // Down.
    king_lookup[index] |= (king & ~Bitboard::rank_1 & ~Bitboard::file_A) >> 9;  // Down left.
  }
  return king_lookup;
})();

Bitboard King::attacks(Bitboard square) { return king_lookup[square.to_index()]; }