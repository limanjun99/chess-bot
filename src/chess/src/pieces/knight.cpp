#include "pieces/knight.h"

#include <array>

using namespace chess;

constexpr std::array<Bitboard, 64> knight_lookup = ([]() {
  std::array<Bitboard, 64> knight_lookup{};
  for (int index = 0; index < 64; index++) {
    const Bitboard knight{Bitboard::from_index(index)};
    knight_lookup[index] |= (knight & ~Bitboard::rank_8 & ~Bitboard::rank_7 & ~Bitboard::file_H) << 17;  // Up up right.
    knight_lookup[index] |= (knight & ~Bitboard::rank_8 & ~Bitboard::rank_7 & ~Bitboard::file_A) << 15;  // Up up left.
    knight_lookup[index] |= (knight & ~Bitboard::rank_8 & ~Bitboard::file_H & ~Bitboard::file_G)
                            << 10;  // Up right right.
    knight_lookup[index] |= (knight & ~Bitboard::rank_8 & ~Bitboard::file_A & ~Bitboard::file_B) << 6;  // Up left left.
    knight_lookup[index] |=
        (knight & ~Bitboard::rank_1 & ~Bitboard::file_H & ~Bitboard::file_G) >> 6;  // Down right right.
    knight_lookup[index] |=
        (knight & ~Bitboard::rank_1 & ~Bitboard::file_A & ~Bitboard::file_B) >> 10;  // Down left left.
    knight_lookup[index] |=
        (knight & ~Bitboard::rank_1 & ~Bitboard::rank_2 & ~Bitboard::file_H) >> 15;  // Down down right.
    knight_lookup[index] |=
        (knight & ~Bitboard::rank_1 & ~Bitboard::rank_2 & ~Bitboard::file_A) >> 17;  // Down down left.
  }
  return knight_lookup;
})();

Bitboard Knight::attacks(Bitboard square) { return knight_lookup[square.to_index()]; }