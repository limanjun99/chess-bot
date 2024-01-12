#pragma once

#include <vector>

#include "move.h"

// A container to store moves in.
class MoveContainer {
public:
  MoveContainer();

  // Construct and add a new move.
  template <class... Args>
  void emplace_back(Args&&... args);

  // Returns true if there are no moves in the container.
  bool empty() const;

  // Number of moves in the container.
  size_t size() const;

  // Get the move at the given index.
  const Move& operator[](size_t index) const;
  Move& operator[](size_t index);

private:
  // Note that the moves are purposely stored in an array instead of a heap-allocating container for performance.
  // Max number of legal moves in any position is probably 218 (https://www.chessprogramming.org/Chess_Position).
  Move moves[218];
  size_t size_;
};

inline MoveContainer::MoveContainer() : size_{0} {}

template <class... Args>
inline void MoveContainer::emplace_back(Args&&... args) {
  moves[size_] = Move(args...);
  size_++;
}

inline bool MoveContainer::empty() const { return size_ == 0; }

inline size_t MoveContainer::size() const { return size_; }

inline const Move& MoveContainer::operator[](size_t index) const { return moves[index]; }
inline Move& MoveContainer::operator[](size_t index) { return moves[index]; }
