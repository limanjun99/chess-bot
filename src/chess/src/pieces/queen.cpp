#include "pieces/queen.h"

#include "pieces/bishop.h"
#include "pieces/rook.h"

u64 Queen::attacks(u64 square, u64 occupancy) {
  return Bishop::attacks(square, occupancy) | Rook::attacks(square, occupancy);
}