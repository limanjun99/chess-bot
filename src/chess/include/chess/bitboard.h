#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

namespace chess {

struct Bitboard {
public:
  uint64_t value;

  // Constructs a bitboard from algberaic notation (e.g. 'a1').
  static constexpr Bitboard from_algebraic(std::string_view algebraic);

  // Constructs a bitboard from a square index (0-63).
  static constexpr Bitboard from_index(int index);

  // Constructs a bitboard from a (y, x) coordinate.
  static constexpr Bitboard from_coordinate(int y, int x);

  constexpr Bitboard operator|(const Bitboard& bitboard) const;
  constexpr Bitboard operator&(const Bitboard& bitboard) const;
  constexpr Bitboard operator^(const Bitboard& bitboard) const;
  constexpr Bitboard operator~() const;
  constexpr Bitboard operator<<(unsigned int shift) const;
  constexpr Bitboard operator>>(unsigned int shift) const;
  constexpr Bitboard& operator|=(const Bitboard& bitboard);
  constexpr Bitboard& operator&=(const Bitboard& bitboard);
  constexpr Bitboard& operator^=(const Bitboard& bitboard);
  constexpr Bitboard& operator<<=(unsigned int shift);
  constexpr Bitboard& operator>>=(unsigned int shift);
  constexpr bool operator==(const Bitboard& bitboard) const;
  constexpr bool operator!=(const Bitboard& bitboard) const;
  constexpr explicit operator bool() const;
  constexpr explicit operator uint64_t() const;

  // Returns a bitboard of squares from this bitboard to the `to` bitboard.
  // Both bitboards must have exactly one bit set, and be horizontally / vertically / diagonally apart,
  // otherwise it is undefined behavior.
  // For example (f = `this`, t = `to`, x = returned bitboard).
  // ........
  // .....t..
  // ....x...
  // ...x....
  // ..x.....
  // .f......
  // ........
  // ........
  constexpr Bitboard until(Bitboard to) const;

  // Returns a bitboard of squares beyond the `to` bitboard, from this bitboard.
  // Both bitboards must have exactly one bit set, and be horizontally / vertically / diagonally apart,
  // otherwise it is undefined behavior.
  // For example (f = `this`, t = `to`, x = returned bitboard).
  // ......x.
  // .....x..
  // ....t...
  // ........
  // ........
  // .f......
  // ........
  // ........
  constexpr Bitboard beyond(Bitboard to) const;

  // Returns a bitboard of squares on the ray to the `to` bitboard, from this bitboard.
  // Both bitboards must have exactly one bit set, otherwise it is undefined behavior.
  // If both bitboards are the same, it is undefined behavior.
  // If the `to` bitboard is not on any horizontal / vertical / diagonal ray, then an empty bitboard is returned.
  // For example (f = `this`, t = `to`, x = returned bitboard (including f and t)).
  // ......x.
  // .....x..
  // ....t...
  // ...x....
  // ..x.....
  // .f......
  // x.......
  // ........
  constexpr Bitboard ray(Bitboard to) const;

  // Counts the number of set bits in this bitboard.
  constexpr int count() const;

  // Converts this bitboard into a 8x8 binary string for visualization.
  constexpr std::string to_string() const;

  // Convert this bitboard into its algebraic notation.
  // This bitboard must have exactly one bit set, otherwise it is undefined behavior.
  constexpr std::string to_algebraic() const;

  // Returns the (y, x) coordinate of the set bit in this bitboard.
  // This bitboard must have exactly one bit set, otherwise it is undefined behavior.
  constexpr std::pair<int, int> to_coordinate() const;

  // Returns the index of the set bit in this bitboard.
  // This bitboard must have exactly one bit set, otherwise it is undefined behavior.
  constexpr int to_index() const;

  // Returns the least significant bit of this bitboard.
  constexpr Bitboard lsb() const;

  // Iterates over all set bits in this bitboard.
  // Usage: `for (const Bitboard bit : bitboard.iterate())`.
  constexpr auto iterate() const;

  // Iterates over all subsets (including empty bitboard) of set bits in this bitboard.
  // Usage: `for (const Bitboard subset : bitboard.iterate_subsets())`.
  constexpr auto iterate_subsets() const;

  static const Bitboard A1;
  static const Bitboard B1;
  static const Bitboard C1;
  static const Bitboard D1;
  static const Bitboard E1;
  static const Bitboard F1;
  static const Bitboard G1;
  static const Bitboard H1;
  static const Bitboard A2;
  static const Bitboard B2;
  static const Bitboard C2;
  static const Bitboard D2;
  static const Bitboard E2;
  static const Bitboard F2;
  static const Bitboard G2;
  static const Bitboard H2;
  static const Bitboard A3;
  static const Bitboard B3;
  static const Bitboard C3;
  static const Bitboard D3;
  static const Bitboard E3;
  static const Bitboard F3;
  static const Bitboard G3;
  static const Bitboard H3;
  static const Bitboard A4;
  static const Bitboard B4;
  static const Bitboard C4;
  static const Bitboard D4;
  static const Bitboard E4;
  static const Bitboard F4;
  static const Bitboard G4;
  static const Bitboard H4;
  static const Bitboard A5;
  static const Bitboard B5;
  static const Bitboard C5;
  static const Bitboard D5;
  static const Bitboard E5;
  static const Bitboard F5;
  static const Bitboard G5;
  static const Bitboard H5;
  static const Bitboard A6;
  static const Bitboard B6;
  static const Bitboard C6;
  static const Bitboard D6;
  static const Bitboard E6;
  static const Bitboard F6;
  static const Bitboard G6;
  static const Bitboard H6;
  static const Bitboard A7;
  static const Bitboard B7;
  static const Bitboard C7;
  static const Bitboard D7;
  static const Bitboard E7;
  static const Bitboard F7;
  static const Bitboard G7;
  static const Bitboard H7;
  static const Bitboard A8;
  static const Bitboard B8;
  static const Bitboard C8;
  static const Bitboard D8;
  static const Bitboard E8;
  static const Bitboard F8;
  static const Bitboard G8;
  static const Bitboard H8;

  static const Bitboard rank_1;
  static const Bitboard rank_2;
  static const Bitboard rank_3;
  static const Bitboard rank_4;
  static const Bitboard rank_5;
  static const Bitboard rank_6;
  static const Bitboard rank_7;
  static const Bitboard rank_8;

  static const Bitboard file_A;
  static const Bitboard file_B;
  static const Bitboard file_C;
  static const Bitboard file_D;
  static const Bitboard file_E;
  static const Bitboard file_F;
  static const Bitboard file_G;
  static const Bitboard file_H;

  // Ranks from 1 to 8 (index 0 to 7).
  static const Bitboard rank[8];
  // Files from A to H (index 0 to 7).
  static const Bitboard file[8];
  // Bitboards of forward slashes (/) arranged top-down and left-right,
  // where index 0 is the top left square and index 15 is the bottom right square.
  static const Bitboard forward_slash_diagonal[15];
  // Bitboards of backward slashes (\) arranged bottom-up and left-right.
  // where index 0 is the bottom left square and index 15 is the top right square.
  static const Bitboard backward_slash_diagonal[15];

  static const Bitboard full;
  static const Bitboard empty;

private:
  // This generator enables ranged-based for loops over bitboards (e.g. for Bitboard::iterate).
  // Note that Generator::end() is intentionally useless (kinda hacky), because using a More function to check
  // for end of generation gives more flexibility (e.g. to allow Bitboard::iterate_subset to terminate
  // after reaching 0, instead of upon reaching 0).
  template <typename State, typename Getter, typename Progress, typename More>
  class Generator {
  public:
    static constexpr Getter getter{};
    static constexpr Progress progress{};
    static constexpr More more{};

    class Iterator {
    public:
      constexpr Iterator(State& state) : state{state} {}
      constexpr Iterator& operator++() {
        progress(state);
        return *this;
      }
      constexpr Bitboard operator*() const { return getter(state); }
      constexpr bool operator!=(const Iterator&) const { return more(state); }

    private:
      State& state;
    };

    constexpr Generator(State state) : state{state} {}
    constexpr Iterator begin() { return Iterator{state}; }
    constexpr Iterator end() { return Iterator{state}; }

  private:
    State state;
  };
};

// ========== IMPLEMENTATIONS ==========

constexpr Bitboard Bitboard::from_algebraic(std::string_view algebraic) {
  int x{static_cast<int>(algebraic[0] - 'a')};
  int y{static_cast<int>(algebraic[1] - '1')};
  return Bitboard{uint64_t{1} << (y * 8 + x)};
}

constexpr Bitboard Bitboard::from_index(int index) { return Bitboard{uint64_t{1} << index}; }

constexpr Bitboard Bitboard::from_coordinate(int y, int x) { return Bitboard{uint64_t{1} << (y * 8 + x)}; }

constexpr Bitboard Bitboard::operator|(const Bitboard& bitboard) const { return Bitboard{value | bitboard.value}; }
constexpr Bitboard Bitboard::operator&(const Bitboard& bitboard) const { return Bitboard{value & bitboard.value}; }
constexpr Bitboard Bitboard::operator^(const Bitboard& bitboard) const { return Bitboard{value ^ bitboard.value}; }
constexpr Bitboard Bitboard::operator~() const { return Bitboard{~value}; }
constexpr Bitboard Bitboard::operator<<(unsigned int shift) const { return Bitboard{value << shift}; }
constexpr Bitboard Bitboard::operator>>(unsigned int shift) const { return Bitboard{value >> shift}; }
constexpr Bitboard& Bitboard::operator|=(const Bitboard& bitboard) {
  value |= bitboard.value;
  return *this;
}
constexpr Bitboard& Bitboard::operator&=(const Bitboard& bitboard) {
  value &= bitboard.value;
  return *this;
}
constexpr Bitboard& Bitboard::operator^=(const Bitboard& bitboard) {
  value ^= bitboard.value;
  return *this;
}
constexpr Bitboard& Bitboard::operator<<=(unsigned int shift) {
  value <<= shift;
  return *this;
}
constexpr Bitboard& Bitboard::operator>>=(unsigned int shift) {
  value >>= shift;
  return *this;
}
constexpr bool Bitboard::operator==(const Bitboard& bitboard) const { return value == bitboard.value; }
constexpr bool Bitboard::operator!=(const Bitboard& bitboard) const { return value != bitboard.value; }
constexpr Bitboard::operator bool() const { return value != 0; }
constexpr Bitboard::operator uint64_t() const { return value; }

namespace detail::bitboard {
constexpr std::array<std::array<Bitboard, 64>, 64> between_array = []() {
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
        between_array[from_index][to_index] = between_array[from_index][prev_index] | Bitboard::from_index(prev_index);
      }
    }
  }
  return between_array;
}();
}
constexpr Bitboard Bitboard::until(Bitboard to) const {
  const int from_index{to_index()};
  const int to_index{to.to_index()};
  return detail::bitboard::between_array[from_index][to_index];
}

namespace detail::bitboard {
constexpr std::array<std::array<Bitboard, 64>, 64> beyond_array = []() {
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
}
constexpr Bitboard Bitboard::beyond(Bitboard to) const {
  const int from_index{to_index()};
  const int to_index{to.to_index()};
  return detail::bitboard::beyond_array[from_index][to_index];
}

constexpr Bitboard Bitboard::ray(Bitboard to) const {
  const auto [fy, fx] = to_coordinate();
  const auto [ty, tx] = to.to_coordinate();
  if (fy == ty) return Bitboard::rank[fy];  // Horizontal ray.
  if (fx == tx) return Bitboard::file[fx];  // Vertical ray.
  const int dy{ty - fy};
  const int dx{tx - fx};
  if (std::abs(dy) != std::abs(dx)) return Bitboard::empty;            // Not on same ray.
  if (dy == dx) return Bitboard::forward_slash_diagonal[fx - fy + 7];  // On diagonal (forward slash /) ray.
  else return Bitboard::backward_slash_diagonal[fx + fy];              // On diagonal (backward slash /) ray.
}

constexpr int Bitboard::count() const { return __builtin_popcountll(value); }

constexpr std::string Bitboard::to_string() const {
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

constexpr std::string Bitboard::to_algebraic() const {
  std::string algebraic;
  algebraic.reserve(2);
  const auto [rank, file] = to_coordinate();
  algebraic += static_cast<char>(file + 'a');
  algebraic += static_cast<char>(rank + '1');
  return algebraic;
}

constexpr std::pair<int, int> Bitboard::to_coordinate() const {
  int index{to_index()};
  return {index / 8, index % 8};
}

constexpr int Bitboard::to_index() const { return __builtin_ctzll(value); }

constexpr Bitboard Bitboard::lsb() const { return Bitboard{value & -value}; }

constexpr auto Bitboard::iterate() const {
  constexpr auto getter{[](const uint64_t& state) { return Bitboard{state & -state}; }};
  constexpr auto progress{[](uint64_t& state) { state ^= state & -state; }};
  constexpr auto more{[](const uint64_t& state) { return state; }};
  return Generator<uint64_t, decltype(getter), decltype(progress), decltype(more)>(value);
}

constexpr auto Bitboard::iterate_subsets() const {
  struct State {
    uint64_t set;
    uint64_t current_subset;
    bool done;
  };
  constexpr auto getter{[](const State& state) { return Bitboard{state.current_subset}; }};
  constexpr auto progress{[](State& state) {
    if (!state.current_subset) state.done = true;
    else state.current_subset = (state.current_subset - 1) & state.set;
  }};
  constexpr auto more{[](const State& state) { return !state.done; }};
  return Generator<State, decltype(getter), decltype(progress), decltype(more)>(State{value, value, false});
}

constexpr Bitboard Bitboard::A1{Bitboard::from_index(0)};
constexpr Bitboard Bitboard::B1{Bitboard::from_index(1)};
constexpr Bitboard Bitboard::C1{Bitboard::from_index(2)};
constexpr Bitboard Bitboard::D1{Bitboard::from_index(3)};
constexpr Bitboard Bitboard::E1{Bitboard::from_index(4)};
constexpr Bitboard Bitboard::F1{Bitboard::from_index(5)};
constexpr Bitboard Bitboard::G1{Bitboard::from_index(6)};
constexpr Bitboard Bitboard::H1{Bitboard::from_index(7)};
constexpr Bitboard Bitboard::A2{Bitboard::from_index(8)};
constexpr Bitboard Bitboard::B2{Bitboard::from_index(9)};
constexpr Bitboard Bitboard::C2{Bitboard::from_index(10)};
constexpr Bitboard Bitboard::D2{Bitboard::from_index(11)};
constexpr Bitboard Bitboard::E2{Bitboard::from_index(12)};
constexpr Bitboard Bitboard::F2{Bitboard::from_index(13)};
constexpr Bitboard Bitboard::G2{Bitboard::from_index(14)};
constexpr Bitboard Bitboard::H2{Bitboard::from_index(15)};
constexpr Bitboard Bitboard::A3{Bitboard::from_index(16)};
constexpr Bitboard Bitboard::B3{Bitboard::from_index(17)};
constexpr Bitboard Bitboard::C3{Bitboard::from_index(18)};
constexpr Bitboard Bitboard::D3{Bitboard::from_index(19)};
constexpr Bitboard Bitboard::E3{Bitboard::from_index(20)};
constexpr Bitboard Bitboard::F3{Bitboard::from_index(21)};
constexpr Bitboard Bitboard::G3{Bitboard::from_index(22)};
constexpr Bitboard Bitboard::H3{Bitboard::from_index(23)};
constexpr Bitboard Bitboard::A4{Bitboard::from_index(24)};
constexpr Bitboard Bitboard::B4{Bitboard::from_index(25)};
constexpr Bitboard Bitboard::C4{Bitboard::from_index(26)};
constexpr Bitboard Bitboard::D4{Bitboard::from_index(27)};
constexpr Bitboard Bitboard::E4{Bitboard::from_index(28)};
constexpr Bitboard Bitboard::F4{Bitboard::from_index(29)};
constexpr Bitboard Bitboard::G4{Bitboard::from_index(30)};
constexpr Bitboard Bitboard::H4{Bitboard::from_index(31)};
constexpr Bitboard Bitboard::A5{Bitboard::from_index(32)};
constexpr Bitboard Bitboard::B5{Bitboard::from_index(33)};
constexpr Bitboard Bitboard::C5{Bitboard::from_index(34)};
constexpr Bitboard Bitboard::D5{Bitboard::from_index(35)};
constexpr Bitboard Bitboard::E5{Bitboard::from_index(36)};
constexpr Bitboard Bitboard::F5{Bitboard::from_index(37)};
constexpr Bitboard Bitboard::G5{Bitboard::from_index(38)};
constexpr Bitboard Bitboard::H5{Bitboard::from_index(39)};
constexpr Bitboard Bitboard::A6{Bitboard::from_index(40)};
constexpr Bitboard Bitboard::B6{Bitboard::from_index(41)};
constexpr Bitboard Bitboard::C6{Bitboard::from_index(42)};
constexpr Bitboard Bitboard::D6{Bitboard::from_index(43)};
constexpr Bitboard Bitboard::E6{Bitboard::from_index(44)};
constexpr Bitboard Bitboard::F6{Bitboard::from_index(45)};
constexpr Bitboard Bitboard::G6{Bitboard::from_index(46)};
constexpr Bitboard Bitboard::H6{Bitboard::from_index(47)};
constexpr Bitboard Bitboard::A7{Bitboard::from_index(48)};
constexpr Bitboard Bitboard::B7{Bitboard::from_index(49)};
constexpr Bitboard Bitboard::C7{Bitboard::from_index(50)};
constexpr Bitboard Bitboard::D7{Bitboard::from_index(51)};
constexpr Bitboard Bitboard::E7{Bitboard::from_index(52)};
constexpr Bitboard Bitboard::F7{Bitboard::from_index(53)};
constexpr Bitboard Bitboard::G7{Bitboard::from_index(54)};
constexpr Bitboard Bitboard::H7{Bitboard::from_index(55)};
constexpr Bitboard Bitboard::A8{Bitboard::from_index(56)};
constexpr Bitboard Bitboard::B8{Bitboard::from_index(57)};
constexpr Bitboard Bitboard::C8{Bitboard::from_index(58)};
constexpr Bitboard Bitboard::D8{Bitboard::from_index(59)};
constexpr Bitboard Bitboard::E8{Bitboard::from_index(60)};
constexpr Bitboard Bitboard::F8{Bitboard::from_index(61)};
constexpr Bitboard Bitboard::G8{Bitboard::from_index(62)};
constexpr Bitboard Bitboard::H8{Bitboard::from_index(63)};

constexpr Bitboard Bitboard::rank_1{A1 | B1 | C1 | D1 | E1 | F1 | G1 | H1};
constexpr Bitboard Bitboard::rank_2{rank_1 << 8};
constexpr Bitboard Bitboard::rank_3{rank_1 << 16};
constexpr Bitboard Bitboard::rank_4{rank_1 << 24};
constexpr Bitboard Bitboard::rank_5{rank_1 << 32};
constexpr Bitboard Bitboard::rank_6{rank_1 << 40};
constexpr Bitboard Bitboard::rank_7{rank_1 << 48};
constexpr Bitboard Bitboard::rank_8{rank_1 << 56};

constexpr Bitboard Bitboard::file_A{A1 | A2 | A3 | A4 | A5 | A6 | A7 | A8};
constexpr Bitboard Bitboard::file_B{file_A << 1};
constexpr Bitboard Bitboard::file_C{file_A << 2};
constexpr Bitboard Bitboard::file_D{file_A << 3};
constexpr Bitboard Bitboard::file_E{file_A << 4};
constexpr Bitboard Bitboard::file_F{file_A << 5};
constexpr Bitboard Bitboard::file_G{file_A << 6};
constexpr Bitboard Bitboard::file_H{file_A << 7};

constexpr Bitboard Bitboard::rank[8]{rank_1, rank_2, rank_3, rank_4, rank_5, rank_6, rank_7, rank_8};
constexpr Bitboard Bitboard::file[8]{file_A, file_B, file_C, file_D, file_E, file_F, file_G, file_H};

constexpr Bitboard Bitboard::forward_slash_diagonal[15]{A8,
                                                        A7 | B8,
                                                        A6 | B7 | C8,
                                                        A5 | B6 | C7 | D8,
                                                        A4 | B5 | C6 | D7 | E8,
                                                        A3 | B4 | C5 | D6 | E7 | F8,
                                                        A2 | B3 | C4 | D5 | E6 | F7 | G8,
                                                        A1 | B2 | C3 | D4 | E5 | F6 | G7 | H8,
                                                        B1 | C2 | D3 | E4 | F5 | G6 | H7,
                                                        C1 | D2 | E3 | F4 | G5 | H6,
                                                        D1 | E2 | F3 | G4 | H5,
                                                        E1 | F2 | G3 | H4,
                                                        F1 | G2 | H3,
                                                        G1 | H2,
                                                        H1};
constexpr Bitboard Bitboard::backward_slash_diagonal[15]{A1,
                                                         A2 | B1,
                                                         A3 | B2 | C1,
                                                         A4 | B3 | C2 | D1,
                                                         A5 | B4 | C3 | D2 | E1,
                                                         A6 | B5 | C4 | D3 | E2 | F1,
                                                         A7 | B6 | C5 | D4 | E3 | F2 | G1,
                                                         A8 | B7 | C6 | D5 | E4 | F3 | G2 | H1,
                                                         B8 | C7 | D6 | E5 | F4 | G3 | H2,
                                                         C8 | D7 | E6 | F5 | G4 | H3,
                                                         D8 | E7 | F6 | G5 | H4,
                                                         E8 | F7 | G6 | H5,
                                                         F8 | G7 | H6,
                                                         G8 | H7,
                                                         H8};

constexpr Bitboard Bitboard::full{~Bitboard{0}};
constexpr Bitboard Bitboard::empty{Bitboard{0}};

}  // namespace chess