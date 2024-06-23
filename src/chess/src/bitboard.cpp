#include "bitboard.h"

#include <array>

using namespace chess;

Bitboard Bitboard::from_algebraic(std::string_view algebraic) {
  int x{static_cast<int>(algebraic[0] - 'a')};
  int y{static_cast<int>(algebraic[1] - '1')};
  return Bitboard{uint64_t{1} << (y * 8 + x)};
}

Bitboard Bitboard::until(Bitboard to) const {
  static constexpr std::array<std::array<Bitboard, 64>, 64> between_array = []() {
    std::array<std::array<Bitboard, 64>, 64> between_array{};
    const std::array<std::pair<int, int>, 8> directions{
        {{0, 1}, {0, -1}, {1, 0}, {-1, 0}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}}};
    for (int from_index{0}; from_index < 64; from_index++) {
      const int from_y{from_index / 8};
      const int from_x{from_index % 8};
      for (int distance{2}; distance < 8; distance++) {
        for (const auto& [delta_y, delta_x] : directions) {
          const int to_y{from_y + distance * delta_y};
          const int to_x{from_x + distance * delta_x};
          const int to_index{to_y * 8 + to_x};
          if (to_x < 0 || to_x >= 8 || to_y < 0 || to_y >= 8) continue;
          const int prev_y{from_y + (distance - 1) * delta_y};
          const int prev_x{from_x + (distance - 1) * delta_x};
          const int prev_index{prev_y * 8 + prev_x};
          between_array[from_index][to_index] =
              between_array[from_index][prev_index] | Bitboard::from_index(prev_index);
        }
      }
    }
    return between_array;
  }();

  const int from_index{to_index()};
  const int to_index{to.to_index()};
  return between_array[from_index][to_index];
}

Bitboard Bitboard::beyond(Bitboard to) const {
  static constexpr std::array<std::array<Bitboard, 64>, 64> beyond_array = []() {
    std::array<std::array<Bitboard, 64>, 64> beyond_array{};
    const std::array<std::pair<int, int>, 8> directions{
        {{0, 1}, {0, -1}, {1, 0}, {-1, 0}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}}};
    for (int from_index{0}; from_index < 64; from_index++) {
      const int from_y{from_index / 8};
      const int from_x{from_index % 8};
      for (int distance{7}; distance > 0; distance--) {
        for (const auto& [delta_y, delta_x] : directions) {
          const int to_y{from_y + distance * delta_y};
          const int to_x{from_x + distance * delta_x};
          const int to_index{to_y * 8 + to_x};
          const int prev_y{from_y + (distance + 1) * delta_y};
          const int prev_x{from_x + (distance + 1) * delta_x};
          const int prev_index{prev_y * 8 + prev_x};
          if (prev_x < 0 || prev_x >= 8 || prev_y < 0 || prev_y >= 8) continue;
          beyond_array[from_index][to_index] = beyond_array[from_index][prev_index] | Bitboard::from_index(prev_index);
        }
      }
    }
    return beyond_array;
  }();

  const int from_index{to_index()};
  const int to_index{to.to_index()};
  return beyond_array[from_index][to_index];
}

std::string Bitboard::to_string() const {
  std::string s;
  s.reserve(8 * 8 + 7);
  for (int y{7}; y >= 0; y--) {
    for (int x{0}; x < 8; x++) {
      s += (*this & Bitboard::from_coordinate(y, x)) ? '1' : '0';
    }
    if (y > 0) s += '\n';
  }
  return s;
}

std::string Bitboard::to_algebraic() const {
  std::string algebraic;
  algebraic.reserve(2);
  const auto [rank, file] = to_coordinate();
  algebraic += static_cast<char>(file + 'a');
  algebraic += static_cast<char>(rank + '1');
  return algebraic;
}