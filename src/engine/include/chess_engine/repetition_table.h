#pragma once

#include <cstdint>

#include "board_hash.h"

class RepetitionTable {
public:
  RepetitionTable();

  // Add the hash to the list of seen positions.
  void add(u64 hash);

  // Check if adding the given hash leads to a draw.
  bool is_draw_if_add(u64 hash);

  ~RepetitionTable();

private:
  int size;
  u64* keys;
  int8_t* counts;
};