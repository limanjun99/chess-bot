#pragma once

#include "board.h"
#include "move.h"
#include "piece.h"

namespace chess {

namespace uci {

// Construct a Move from a uci string (assumed to be valid) and its corresponding Board.
Move move(std::string_view uci_move, const Board& board);

}  // namespace uci

// ========== IMPLEMENTATIONS ==========

inline Move uci::move(std::string_view uci_move, const Board& board) {
  const Bitboard from{Bitboard::from_algebraic(uci_move.substr(0, 2))};
  const Bitboard to{Bitboard::from_algebraic(uci_move.substr(2, 2))};
  const PieceType captured_piece{board.opp_player().piece_at(to)};
  if (uci_move.size() == 5) {
    const PieceType promotion_piece{piece::from_char(uci_move[4])};
    return Move::promotion(from, to, promotion_piece, captured_piece);
  } else {
    const PieceType from_piece{board.cur_player().piece_at(from)};
    return Move::move(from, to, from_piece, captured_piece);
  }
}

}  // namespace chess