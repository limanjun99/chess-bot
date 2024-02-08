#include "pieces/king.h"

#include <array>

constexpr std::array<u64, 64> king_lookup = ([]() {
  using namespace bitboard;
  std::array<u64, 64> king_lookup{};
  for (int index = 0; index < 64; index++) {
    const u64 king = u64(1) << index;
    king_lookup[index] |= (king & ~RANK_8 & ~FILE_H) << 9;  // Up right.
    king_lookup[index] |= (king & ~RANK_8) << 8;            // Up.
    king_lookup[index] |= (king & ~RANK_8 & ~FILE_A) << 7;  // Up left.
    king_lookup[index] |= (king & ~FILE_H) << 1;            // Right.
    king_lookup[index] |= (king & ~FILE_A) >> 1;            // Left.
    king_lookup[index] |= (king & ~RANK_1 & ~FILE_H) >> 7;  // Down right.
    king_lookup[index] |= (king & ~RANK_1) >> 8;            // Down.
    king_lookup[index] |= (king & ~RANK_1 & ~FILE_A) >> 9;  // Down left.
  }
  return king_lookup;
})();

u64 King::attacks(u64 square) { return king_lookup[bit::to_index(square)]; }