#pragma once

#include <cstdint>

class RepetitionTable {
public:
  RepetitionTable();

  // Add the hash to the list of seen positions.
  void add(uint64_t hash);

  // Check if adding the given hash leads to a draw.
  bool is_draw_if_add(uint64_t hash);

  ~RepetitionTable();

private:
  int size;
  uint64_t* keys;
  int8_t* counts;
};