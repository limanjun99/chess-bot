#include "bitboard.h"

#include <array>

namespace {
constexpr std::array<std::array<u64, 64>, 64> between_array = []() {
  std::array<std::array<u64, 64>, 64> between_array{{{0}}};
  std::array<std::pair<int, int>, 8> directions{{{0, 1}, {0, -1}, {1, 0}, {-1, 0}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}}};
  for (int from_index = 0; from_index < 64; from_index++) {
    const int from_y = from_index / 8;
    const int from_x = from_index % 8;
    for (int distance = 2; distance < 8; distance++) {
      for (auto [delta_y, delta_x] : directions) {
        const int to_y = from_y + distance * delta_y;
        const int to_x = from_x + distance * delta_x;
        const int to_index = to_y * 8 + to_x;
        if (to_x < 0 || to_x >= 8 || to_y < 0 || to_y >= 8) continue;
        const int prev_y = to_y - delta_y;
        const int prev_x = to_x - delta_x;
        const int prev_index = prev_y * 8 + prev_x;
        between_array[from_index][to_index] = between_array[from_index][prev_index] | u64(1) << prev_index;
      }
    }
  }
  return between_array;
}();
}  // namespace
u64 bitboard::between(u64 from, u64 to) {
  const int from_index = bit::to_index(from);
  const int to_index = bit::to_index(to);
  return between_array[from_index][to_index];
}

namespace {
constexpr std::array<std::array<u64, 64>, 64> beyond_array = []() {
  std::array<std::array<u64, 64>, 64> beyond_array{{{0}}};
  std::array<std::pair<int, int>, 8> directions{{{0, 1}, {0, -1}, {1, 0}, {-1, 0}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}}};
  for (int from_index = 0; from_index < 64; from_index++) {
    const int from_y = from_index / 8;
    const int from_x = from_index % 8;
    for (int distance = 7; distance > 0; distance--) {
      for (auto [delta_y, delta_x] : directions) {
        const int to_y = from_y + distance * delta_y;
        const int to_x = from_x + distance * delta_x;
        const int to_index = to_y * 8 + to_x;
        const int prev_y = to_y + delta_y;
        const int prev_x = to_x + delta_x;
        const int prev_index = prev_y * 8 + prev_x;
        if (prev_x < 0 || prev_x >= 8 || prev_y < 0 || prev_y >= 8) continue;
        beyond_array[from_index][to_index] = beyond_array[from_index][prev_index] | u64(1) << prev_index;
      }
    }
  }
  return beyond_array;
}();
}  // namespace
u64 bitboard::beyond(u64 from, u64 to) {
  const int from_index = bit::to_index(from);
  const int to_index = bit::to_index(to);
  return beyond_array[from_index][to_index];
}

std::string bitboard::to_string(u64 bitboard) {
  std::string s;
  s.reserve(8 * 8 + 7);
  for (int y = 7; y >= 0; y--) {
    for (int x = 0; x < 8; x++) {
      s += (bitboard & (u64(1) << (y * 8 + x))) ? '1' : '0';
    }
    if (y > 0) s += '\n';
  }
  return s;
}

u64 bit::from_algebraic(std::string_view algebraic) {
  int x = static_cast<int>(algebraic[0] - 'a');
  int y = static_cast<int>(algebraic[1] - '1');
  return u64(1) << (y * 8 + x);
}

std::string bit::to_algebraic(u64 bit) {
  std::string algebraic;
  int index = bit::to_index(bit);
  int rank = index / 8;
  int file = index % 8;
  algebraic += static_cast<char>(file + 'a');
  algebraic += static_cast<char>(rank + '1');
  return algebraic;
}