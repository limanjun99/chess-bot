#include "pieces/queen.h"

#include "pieces/bishop.h"
#include "pieces/rook.h"

using namespace chess;

Bitboard Queen::attacks(Bitboard square, Bitboard occupancy) {
  return Bishop::attacks(square, occupancy) | Rook::attacks(square, occupancy);
}