#pragma once

#include "move.h"

namespace chess {

class MoveContainer {
public:
  constexpr explicit MoveContainer();

  // Add a new move.
  constexpr void push_back(Move move);

  // Returns true if there are no moves.
  constexpr bool empty() const;

  // Returns the number of moves.
  constexpr size_t size() const;

  // Returns a reference to the `index` element.
  constexpr Move& operator[](size_t index);

  // Iterator to beginning of container.
  constexpr Move* begin();

  // Iterator to end of container.
  constexpr Move* end();

private:
  // Maximum number of legal moves in a chess position is 218.
  // (source: https://www.chessprogramming.org/Chess_Position)
  static constexpr size_t maximum_moves{218};

  size_t size_;
  // This is intentionally an array of size 218. Even though most positions have less moves,
  // the following optimization attempts have been benchmarked and are inferior to this implementation.
  //
  // 1. Use a smaller array size, and only heap allocate if there are too many moves.
  // This performed worse even for postions that did not require heap allocation.
  // (likely because push_back and indexing require checks to see if the index exceeds the array).
  //
  // 2. Use some kind of uninitialized memory on the stack (e.g. std::byte[]) to avoid
  // initializing all 218 moves (which std::array does). The performance turned out to be identical, and
  // the added code complexity is just not worth it.
  std::array<Move, maximum_moves> moves;
};

// ========== IMPLEMENTATIONS ==========

constexpr MoveContainer::MoveContainer() : size_{0} {}

constexpr void MoveContainer::push_back(Move move) { moves[size_++] = move; }

constexpr size_t MoveContainer::size() const { return size_; }

constexpr Move& MoveContainer::operator[](size_t index) { return moves[index]; }

constexpr bool MoveContainer::empty() const { return size_ == 0; }

constexpr Move* MoveContainer::begin() { return &moves[0]; }

constexpr Move* MoveContainer::end() { return &moves[size_]; }

}  // namespace chess