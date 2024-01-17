#include "repetition_table.h"

#include "config.h"

RepetitionTable::RepetitionTable() : size{config::repetition_table_size} {
  keys = new u64[size];
  counts = new int8_t[size];
}

void RepetitionTable::add(u64 hash) {
  int index = hash % size;
  while (true) {
    if (keys[index] == hash) {
      counts[index]++;
      break;
    }
    if (keys[index] == 0) {
      keys[index] = hash;
      counts[index] = 1;
      break;
    }
    index = (index + 1) % size;
  }
}

bool RepetitionTable::is_draw_if_add(u64 hash) {
  int index = hash % size;
  while (true) {
    if (keys[index] == 0) {
      return false;
    }
    if (keys[index] == hash) {
      return counts[index] >= 2;
    }
    index = (index + 1) % size;
  }
}

RepetitionTable::~RepetitionTable() {
  delete[] keys;
  delete[] counts;
}
