#pragma once

#include <type_traits>
#include <vector>

#include "move.h"

namespace chess {

//! TODO: This is literally a vector.
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

  ~MoveContainer();

private:
  // Use malloc instead of new to avoid overhead of default constructing all Move objects.
  // Max number of legal moves in any position is probably 218 (https://www.chessprogramming.org/Chess_Position).
  Move* moves = static_cast<Move*>(malloc(sizeof(Move) * 218));
  size_t size_;
};

inline MoveContainer::MoveContainer() : size_{0} {}

template <class... Args>
inline void MoveContainer::emplace_back(Args&&... args) {
  new (&moves[size_]) Move(args...);
  size_++;
}

inline bool MoveContainer::empty() const { return size_ == 0; }

inline size_t MoveContainer::size() const { return size_; }

inline const Move& MoveContainer::operator[](size_t index) const { return moves[index]; }
inline Move& MoveContainer::operator[](size_t index) { return moves[index]; }

inline MoveContainer::~MoveContainer() {
  // Move is trivially destructible, hence we avoid the overhead of calling the destructor on each object.
  static_assert(std::is_trivially_destructible_v<Move>);
  free(moves);
}

}  // namespace chess