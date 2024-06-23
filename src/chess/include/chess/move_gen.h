#pragma once

#include "board.h"
#include "move_container.h"

namespace chess {

namespace move_gen {
// Generate a list of all legal moves.
MoveContainer generate_moves(const Board& board);

// Generate a list of all legal captures and promotions.
MoveContainer generate_quiescence_moves(const Board& board);

// Generate a list of all legal captures, checks and promotions.
MoveContainer generate_quiescence_moves_and_checks(const Board& board);

// Returns true if the current player still has any move to make.
bool has_moves(const Board& board);

// Check if the given square is under attack by the opponent.
bool is_under_attack(const Board& board, Bitboard square);
}  // namespace move_gen

}  // namespace chess