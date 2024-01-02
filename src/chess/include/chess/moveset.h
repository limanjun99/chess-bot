#pragma once

#include <cstdio>
#include <optional>
#include <utility>

#include "bitboard.h"
#include "move.h"
#include "piece.h"

class Board;

class MoveSet {
public:
  MoveSet(const Board& board);

  // Add moves of a piece from a given square to all squares in to_bitboard.
  void add_piece_moves(Piece piece, u64 from, u64 to_bitboard);

  // Applies the next legal move in the moveset and returns the new board (or std::nullopt if no legal moves are left).
  std::optional<std::pair<Move, Board>> apply_next();

private:
  // Represents all possible moves from a single piece.
  struct PieceMoves {
    Piece piece;
    u64 from;
    u64 to_bitboard;
  };

  const Board& board;
  PieceMoves piece_moves[16];  // A player has at most 16 pieces.
  size_t size;                 // Number of PieceMoves in array.
  size_t promotion_flag;       // Used to keep track of iteration through promotion pieces.
};