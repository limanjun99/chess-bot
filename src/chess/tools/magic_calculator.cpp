// This program calculates occupancy masks and magic numbers for sliding piece attacks.

#include <array>
#include <cstdint>
#include <iostream>
#include <map>
#include <random>
#include <utility>
#include <vector>
using u64 = uint64_t;

class MagicBitboard {
public:
  MagicBitboard(int shift, std::array<std::pair<int, int>, 4> directions) : shift{shift}, directions{directions} {}

  // Calculate occupancy masks and magic numbers
  void generate() {
    generate_masks();
    generate_magics();
  }

  // Output information to be copied into the chess library.
  void print() {
    std::cout << "Shift: " << shift << '\n';
    std::cout << "Masks: ";
    for (int i = 0; i < 64; i++) {
      std::cout << masks[i] << "U";
      if (i < 63) std::cout << ", ";
    }
    std::cout << '\n';
    std::cout << "Magics: ";
    for (int i = 0; i < 64; i++) {
      std::cout << magics[i] << "U";
      if (i < 63) std::cout << ", ";
    }
  }

private:
  int shift;
  std::array<std::pair<int, int>, 4> directions;
  u64 masks[64];
  u64 magics[64];
  std::vector<u64> moves[64];

  // Generate occupancy masks.
  void generate_masks() {
    for (int index = 0; index < 64; index++) {
      masks[index] = 0;
      int y = index / 8;
      int x = index % 8;
      for (auto [delta_y, delta_x] : directions) {
        int to_y = y + delta_y;
        int to_x = x + delta_x;
        while (true) {
          int next_y = to_y + delta_y;
          int next_x = to_x + delta_x;
          if (next_y < 0 || next_y >= 8 || next_x < 0 || next_x >= 8) break;
          masks[index] ^= u64(1) << (to_y * 8 + to_x);
          to_y = next_y;
          to_x = next_x;
        }
      }
    }
  }

  // Generate magic numbers.
  void generate_magics() {
    std::mt19937_64 rng{0};
    std::uniform_int_distribution<u64> distrib;

    for (int index = 0; index < 64; index++) {
      int y = index / 8;
      int x = index % 8;

      // Pre-compute moves bitmap for all possible occupancies.
      std::map<u64, u64> moves_bitmap;
      u64 mask = masks[index];
      for (u64 subset = mask;; subset = (subset - 1) & mask) {
        u64 move_bitmap = 0;
        for (auto [delta_y, delta_x] : directions) {
          int to_y = y + delta_y;
          int to_x = x + delta_x;
          while (0 <= to_y && to_y < 8 && 0 <= to_x && to_x < 8) {
            u64 to_bit = u64(1) << (to_y * 8 + to_x);
            move_bitmap ^= to_bit;
            if (to_bit & subset) break;
            to_y += delta_y;
            to_x += delta_x;
          }
        }
        moves_bitmap[subset] = move_bitmap;
        if (subset == 0) break;
      }

      // Randomly generate magic until it is valid
      bool can = false;
      moves[index].resize(1 << (64 - shift));
      while (!can) {
        can = true;
        magics[index] = distrib(rng) & distrib(rng) & distrib(rng);
        fill(moves[index].begin(), moves[index].end(), 0);
        for (auto [occupancy, move_bitmap] : moves_bitmap) {
          u64 hash_index = (magics[index] * occupancy) >> shift;
          if (moves[index][hash_index] == 0) {
            moves[index][hash_index] = move_bitmap;
          } else if (moves[index][hash_index] != move_bitmap) {
            can = false;
            break;
          }
        }
      }
    }
  }
};

int main() {
  // Values for shift (55 for bishop, 52 for rook) were found by increasing them until no magic is generated in
  // reasonable time.

  MagicBitboard bishop_magic{55, {{{-1, -1}, {-1, 1}, {1, -1}, {1, 1}}}};
  bishop_magic.generate();
  std::cout << "========== Bishop magic ==========\n";
  bishop_magic.print();
  std::cout << '\n';

  MagicBitboard rook_magic{52, {{{-1, 0}, {1, 0}, {0, -1}, {0, 1}}}};
  rook_magic.generate();
  std::cout << "========== Rook magic ==========\n";
  rook_magic.print();
}