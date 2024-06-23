#pragma once

#include <cstdint>
#include <string>
#include <string_view>
#include <utility>

namespace chess {

struct Bitboard {
public:
  uint64_t value;

  // Constructs a bitboard from algberaic notation (e.g. 'a1').
  static Bitboard from_algebraic(std::string_view algebraic);

  // Constructs a bitboard from a square index (0-63).
  static constexpr Bitboard from_index(int index) { return Bitboard{uint64_t{1} << index}; }

  // Constructs a bitboard from a (y, x) coordinate.
  static constexpr Bitboard from_coordinate(int y, int x) { return Bitboard{uint64_t{1} << (y * 8 + x)}; }

  constexpr Bitboard operator|(Bitboard bitboard) const { return Bitboard{value | bitboard.value}; }
  constexpr Bitboard operator&(Bitboard bitboard) const { return Bitboard{value & bitboard.value}; }
  constexpr Bitboard operator^(Bitboard bitboard) const { return Bitboard{value ^ bitboard.value}; }
  constexpr Bitboard operator~() const { return Bitboard{~value}; }
  constexpr Bitboard operator<<(unsigned int shift) const { return Bitboard{value << shift}; }
  constexpr Bitboard operator>>(unsigned int shift) const { return Bitboard{value >> shift}; }
  constexpr Bitboard& operator|=(Bitboard bitboard) {
    value |= bitboard.value;
    return *this;
  }
  constexpr Bitboard& operator&=(Bitboard bitboard) {
    value &= bitboard.value;
    return *this;
  }
  constexpr Bitboard& operator^=(Bitboard bitboard) {
    value ^= bitboard.value;
    return *this;
  }
  constexpr Bitboard& operator<<=(unsigned int shift) {
    value <<= shift;
    return *this;
  }
  constexpr Bitboard& operator>>=(unsigned int shift) {
    value >>= shift;
    return *this;
  }
  constexpr bool operator==(Bitboard bitboard) const { return value == bitboard.value; }
  constexpr bool operator!=(Bitboard bitboard) const { return value != bitboard.value; }
  constexpr explicit operator bool() const { return value != 0; }
  constexpr explicit operator uint64_t() const { return value; }

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
  Bitboard until(Bitboard to) const;

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
  Bitboard beyond(Bitboard to) const;

  // Counts the number of set bits in this bitboard.
  constexpr int count() const { return __builtin_popcountll(value); }

  // Converts this bitboard into a 8x8 binary string for visualization.
  std::string to_string() const;

  // Convert this bitboard into its algebraic notation.
  // This bitboard must have exactly one bit set, otherwise it is undefined behavior.
  std::string to_algebraic() const;

  // Returns the (y, x) coordinate of the set bit in this bitboard.
  // This bitboard must have exactly one bit set, otherwise it is undefined behavior.
  constexpr std::pair<int, int> to_coordinate() const {
    int index{to_index()};
    return {index / 8, index % 8};
  }

  // Returns the index of the set bit in this bitboard.
  // This bitboard must have exactly one bit set, otherwise it is undefined behavior.
  constexpr int to_index() const { return __builtin_ctzll(value); }

  // Returns the least significant bit of this bitboard.
  constexpr Bitboard lsb() const { return Bitboard{value & -value}; }

  // This generator enables ranged-based for loops over bitboards (e.g. for Bitboard::iterate).
  // Note that Generator::end() is intentionally useless (kinda hacky), because using a More functor to check
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

  // Iterates over all set bits in this bitboard.
  // Usage: `for (const Bitboard bit : bitboard.iterate())`.
  constexpr auto iterate() const {
    constexpr auto getter{[](const uint64_t& state) { return Bitboard{state & -state}; }};
    constexpr auto progress{[](uint64_t& state) { state ^= state & -state; }};
    constexpr auto more{[](const uint64_t& state) { return state; }};
    return Generator<uint64_t, decltype(getter), decltype(progress), decltype(more)>(value);
  }

  // Iterates over all subsets (including empty bitboard) of set bits in this bitboard.
  // Usage: `for (const Bitboard subset : bitboard.iterate_subsets())`.
  constexpr auto iterate_subsets() const {
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
};

namespace bitboard {

constexpr Bitboard A1{Bitboard::from_index(0)};
constexpr Bitboard B1{Bitboard::from_index(1)};
constexpr Bitboard C1{Bitboard::from_index(2)};
constexpr Bitboard D1{Bitboard::from_index(3)};
constexpr Bitboard E1{Bitboard::from_index(4)};
constexpr Bitboard F1{Bitboard::from_index(5)};
constexpr Bitboard G1{Bitboard::from_index(6)};
constexpr Bitboard H1{Bitboard::from_index(7)};
constexpr Bitboard A2{Bitboard::from_index(8)};
constexpr Bitboard B2{Bitboard::from_index(9)};
constexpr Bitboard C2{Bitboard::from_index(10)};
constexpr Bitboard D2{Bitboard::from_index(11)};
constexpr Bitboard E2{Bitboard::from_index(12)};
constexpr Bitboard F2{Bitboard::from_index(13)};
constexpr Bitboard G2{Bitboard::from_index(14)};
constexpr Bitboard H2{Bitboard::from_index(15)};
constexpr Bitboard A3{Bitboard::from_index(16)};
constexpr Bitboard B3{Bitboard::from_index(17)};
constexpr Bitboard C3{Bitboard::from_index(18)};
constexpr Bitboard D3{Bitboard::from_index(19)};
constexpr Bitboard E3{Bitboard::from_index(20)};
constexpr Bitboard F3{Bitboard::from_index(21)};
constexpr Bitboard G3{Bitboard::from_index(22)};
constexpr Bitboard H3{Bitboard::from_index(23)};
constexpr Bitboard A4{Bitboard::from_index(24)};
constexpr Bitboard B4{Bitboard::from_index(25)};
constexpr Bitboard C4{Bitboard::from_index(26)};
constexpr Bitboard D4{Bitboard::from_index(27)};
constexpr Bitboard E4{Bitboard::from_index(28)};
constexpr Bitboard F4{Bitboard::from_index(29)};
constexpr Bitboard G4{Bitboard::from_index(30)};
constexpr Bitboard H4{Bitboard::from_index(31)};
constexpr Bitboard A5{Bitboard::from_index(32)};
constexpr Bitboard B5{Bitboard::from_index(33)};
constexpr Bitboard C5{Bitboard::from_index(34)};
constexpr Bitboard D5{Bitboard::from_index(35)};
constexpr Bitboard E5{Bitboard::from_index(36)};
constexpr Bitboard F5{Bitboard::from_index(37)};
constexpr Bitboard G5{Bitboard::from_index(38)};
constexpr Bitboard H5{Bitboard::from_index(39)};
constexpr Bitboard A6{Bitboard::from_index(40)};
constexpr Bitboard B6{Bitboard::from_index(41)};
constexpr Bitboard C6{Bitboard::from_index(42)};
constexpr Bitboard D6{Bitboard::from_index(43)};
constexpr Bitboard E6{Bitboard::from_index(44)};
constexpr Bitboard F6{Bitboard::from_index(45)};
constexpr Bitboard G6{Bitboard::from_index(46)};
constexpr Bitboard H6{Bitboard::from_index(47)};
constexpr Bitboard A7{Bitboard::from_index(48)};
constexpr Bitboard B7{Bitboard::from_index(49)};
constexpr Bitboard C7{Bitboard::from_index(50)};
constexpr Bitboard D7{Bitboard::from_index(51)};
constexpr Bitboard E7{Bitboard::from_index(52)};
constexpr Bitboard F7{Bitboard::from_index(53)};
constexpr Bitboard G7{Bitboard::from_index(54)};
constexpr Bitboard H7{Bitboard::from_index(55)};
constexpr Bitboard A8{Bitboard::from_index(56)};
constexpr Bitboard B8{Bitboard::from_index(57)};
constexpr Bitboard C8{Bitboard::from_index(58)};
constexpr Bitboard D8{Bitboard::from_index(59)};
constexpr Bitboard E8{Bitboard::from_index(60)};
constexpr Bitboard F8{Bitboard::from_index(61)};
constexpr Bitboard G8{Bitboard::from_index(62)};
constexpr Bitboard H8{Bitboard::from_index(63)};

constexpr Bitboard RANK_1{A1 | B1 | C1 | D1 | E1 | F1 | G1 | H1};
constexpr Bitboard RANK_2{RANK_1 << 8};
constexpr Bitboard RANK_3{RANK_1 << 16};
constexpr Bitboard RANK_4{RANK_1 << 24};
constexpr Bitboard RANK_5{RANK_1 << 32};
constexpr Bitboard RANK_6{RANK_1 << 40};
constexpr Bitboard RANK_7{RANK_1 << 48};
constexpr Bitboard RANK_8{RANK_1 << 56};

constexpr Bitboard FILE_A{A1 | A2 | A3 | A4 | A5 | A6 | A7 | A8};
constexpr Bitboard FILE_B{FILE_A << 1};
constexpr Bitboard FILE_C{FILE_A << 2};
constexpr Bitboard FILE_D{FILE_A << 3};
constexpr Bitboard FILE_E{FILE_A << 4};
constexpr Bitboard FILE_F{FILE_A << 5};
constexpr Bitboard FILE_G{FILE_A << 6};
constexpr Bitboard FILE_H{FILE_A << 7};

constexpr Bitboard ALL{~Bitboard{0}};
constexpr Bitboard EMPTY{Bitboard{0}};

}  // namespace bitboard

}  // namespace chess