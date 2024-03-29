#pragma once

#include <array>
#include <vector>

#include "bitboard.h"

// Fast lookup of sliding piece attacks.
// Shift, masks and magics should be generated by tools/magic_calculator.cpp
// Details here: https://www.chessprogramming.org/Magic_Bitboards
class MagicBitboard {
public:
  MagicBitboard(std::array<std::pair<int, int>, 4> directions, std::array<int, 64> shifts, std::array<u64, 64> masks,
                std::array<u64, 64> magics);

  // Returns a bitboard of squares attacked by the given bit.
  u64 attacks(u64 bit, u64 occupancy);

private:
  std::array<std::pair<int, int>, 4> directions;  // How the piece moves ({delta y, delta x}).
  std::array<int, 64> shifts;
  std::array<u64, 64> masks;
  std::array<u64, 64> magics;
  std::vector<u64> moves[64];

  // Precompute the moves bitboard for each square.
  void precompute();
};

inline MagicBitboard::MagicBitboard(std::array<std::pair<int, int>, 4> directions, std::array<int, 64> shifts,
                                    std::array<u64, 64> masks, std::array<u64, 64> magics)
    : directions{directions}, shifts{shifts}, masks{masks}, magics{magics} {
  for (int i = 0; i < 64; i++) moves[i].resize(1 << (64 - shifts[i]));
  precompute();
}

inline u64 MagicBitboard::attacks(u64 bit, u64 occupancy) {
  int index = bit::to_index(bit);
  u64 hash = ((occupancy & masks[index]) * magics[index]) >> shifts[index];
  return moves[index][hash];
};

inline void MagicBitboard::precompute() {
  for (int index = 0; index < 64; index++) {
    int y = index / 8;
    int x = index % 8;
    u64 mask = masks[index];
    for (u64 occupancy = mask;; occupancy = (occupancy - 1) & mask) {
      u64 move_bitmap = 0;
      for (auto [delta_y, delta_x] : directions) {
        int to_y = y + delta_y;
        int to_x = x + delta_x;
        while (0 <= to_y && to_y < 8 && 0 <= to_x && to_x < 8) {
          u64 to_bit = u64(1) << (to_y * 8 + to_x);
          move_bitmap ^= to_bit;
          if (to_bit & occupancy) break;
          to_y += delta_y;
          to_x += delta_x;
        }
      }
      u64 hash = (occupancy * magics[index]) >> shifts[index];
      moves[index][hash] = move_bitmap;
      if (occupancy == 0) break;
    }
  }
}