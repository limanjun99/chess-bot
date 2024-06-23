#include "pieces/knight.h"

#include <array>

using namespace chess;

constexpr std::array<Bitboard, 64> knight_lookup = ([]() {
  using namespace bitboard;
  std::array<Bitboard, 64> knight_lookup{};
  for (int index = 0; index < 64; index++) {
    const Bitboard knight{Bitboard::from_index(index)};
    knight_lookup[index] |= (knight & ~RANK_8 & ~RANK_7 & ~FILE_H) << 17;  // Up up right.
    knight_lookup[index] |= (knight & ~RANK_8 & ~RANK_7 & ~FILE_A) << 15;  // Up up left.
    knight_lookup[index] |= (knight & ~RANK_8 & ~FILE_H & ~FILE_G) << 10;  // Up right right.
    knight_lookup[index] |= (knight & ~RANK_8 & ~FILE_A & ~FILE_B) << 6;   // Up left left.
    knight_lookup[index] |= (knight & ~RANK_1 & ~FILE_H & ~FILE_G) >> 6;   // Down right right.
    knight_lookup[index] |= (knight & ~RANK_1 & ~FILE_A & ~FILE_B) >> 10;  // Down left left.
    knight_lookup[index] |= (knight & ~RANK_1 & ~RANK_2 & ~FILE_H) >> 15;  // Down down right.
    knight_lookup[index] |= (knight & ~RANK_1 & ~RANK_2 & ~FILE_A) >> 17;  // Down down left.
  }
  return knight_lookup;
})();

Bitboard Knight::attacks(Bitboard square) { return knight_lookup[square.to_index()]; }