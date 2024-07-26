#pragma once

#include <array>
#include <cstdint>

#include "chess/board.h"
#include "chess/pieces/base_piece.h"

class Evaluation {
public:
  constexpr Evaluation() = default;
  explicit constexpr Evaluation(int16_t value);

  // Returns the given board's evaluation from the perspective of the current player.
  // Higher evaluation is better.
  [[nodiscard]] static Evaluation evaluate(const chess::Board& board);

  static const Evaluation draw;

  // Lowerbound for evaluations (need not be reachable).
  static const Evaluation min;

  // Upperbound for evaluations (need not be reachable).
  static const Evaluation max;

  // Rough value of each piece.
  static const std::array<Evaluation, 6> piece;

  // Represents a winning position found at some `depth`.
  // Higher depths have higher evaluations (because the mate happened in fewer plies).
  static Evaluation winning(int32_t depth);

  // Represents a losing position found at some `depth`.
  // Higher depths have lower evaluations (because the mate happened in fewer plies).
  static Evaluation losing(int32_t depth);

  constexpr auto operator<=>(const Evaluation& other) const = default;
  constexpr Evaluation operator-() const;
  constexpr Evaluation operator+(const Evaluation& other) const;
  constexpr Evaluation& operator+=(const Evaluation& other);

  // Returns true if the evaluation is winning.
  [[nodiscard]] bool is_winning() const;

  // Returns true if the evaluation is losing.
  [[nodiscard]] bool is_losing() const;

  // Returns the successive evaluation (i.e. the smallest evaluation that is larger than this one).
  [[nodiscard]] Evaluation succ() const;

  // Returns the evaluation in centipawns.
  [[nodiscard]] int16_t to_centipawns() const;

private:
  int16_t value;
};

// ===============================================
// =============== IMPLEMENTATIONS ===============
// ===============================================

constexpr Evaluation::Evaluation(int16_t value) : value{value} {}

constexpr Evaluation Evaluation::draw{Evaluation{0}};
constexpr Evaluation Evaluation::min{Evaluation{-30'000}};
constexpr Evaluation Evaluation::max{Evaluation{30'000}};

constexpr std::array<Evaluation, 6> Evaluation::piece{[]() {
  using chess::PieceType;
  std::array<Evaluation, 6> piece{};
  piece[static_cast<size_t>(PieceType::Bishop)] = Evaluation{300};
  piece[static_cast<size_t>(PieceType::King)] = Evaluation{10000};
  piece[static_cast<size_t>(PieceType::Knight)] = Evaluation{300};
  piece[static_cast<size_t>(PieceType::Pawn)] = Evaluation{100};
  piece[static_cast<size_t>(PieceType::Queen)] = Evaluation{900};
  piece[static_cast<size_t>(PieceType::Rook)] = Evaluation{500};
  return piece;
}()};

constexpr Evaluation Evaluation::operator-() const { return Evaluation{static_cast<int16_t>(-value)}; }

constexpr Evaluation Evaluation::operator+(const Evaluation& other) const {
  return Evaluation{static_cast<int16_t>(value + other.value)};
}

constexpr Evaluation& Evaluation::operator+=(const Evaluation& other) {
  value += other.value;
  return *this;
}