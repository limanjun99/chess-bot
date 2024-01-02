#include "move.h"

Move::Move(Piece piece, u64 from, u64 to) : piece{piece}, from{from}, to{to}, promotion_piece{std::nullopt} {}

Move::Move(u64 from, u64 to, Piece promotion_piece)
    : piece{Piece::Pawn}, from{from}, to{to}, promotion_piece{promotion_piece} {}
