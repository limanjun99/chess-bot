#pragma once

#include <concepts>
#include <optional>
#include <string_view>
#include <vector>

#include "bitboard.h"
#include "move_container.h"
#include "player.h"

namespace chess {

template <typename T>
concept IsRepetitionTracker = requires(T repetition_tracker) {
                                { repetition_tracker.is_repetition_draw() } -> std::convertible_to<bool>;
                              };

class Board {
public:
  Board(Player white, Player black, Bitboard en_passant_bit, bool is_white_turn, int16_t halfmove_clock = 0);

  // Starting board of a chess game.
  static Board initial();

  // Construct a board from FEN.
  // https://www.chessprogramming.org/Forsyth-Edwards_Notation
  static Board from_fen(std::string_view fen);

  // Returns a new board that is the result of applying the given move.
  Board apply_move(const Move &move) const;

  // Returns a reference to the current player whose turn it is.
  const Player &cur_player() const;
  Player &cur_player();

  // Generate a list of all legal captures and promotions.
  MoveContainer generate_quiescence_moves() const;

  // Generate a list of all legal captures, checks and promotions.
  MoveContainer generate_quiescence_moves_and_checks() const;

  // Generate a list of all legal moves.
  MoveContainer generate_moves() const;

  // Returns true if the current player still has any move to make.
  bool has_moves() const;

  // Check if the given move is a check. Note that this function is not optimized.
  bool is_a_check(const Move &move) const;

  // Returns true if the current player is in check.
  bool is_in_check() const;

  // Check if the given square is under attack by the opponent.
  bool is_under_attack(Bitboard square) const;

  // Return whether it is white to move.
  inline bool is_white_to_move() const { return is_white_turn; }

  // Returns a reference to the opponent of the current player.
  const Player &opp_player() const;
  Player &opp_player();

  // Return a new board, where the current player skipped their turn.
  Board skip_turn() const;

  // Returns a reference to the white player.
  const Player &get_white() const;

  // Returns a reference to the black player.
  const Player &get_black() const;

  // Returns the bitboard for the en passant bit.
  Bitboard get_en_passant() const;

  // Returns a 8x8 newline delimited string represenation of the board.
  std::string to_string() const;

  // Returns the current turn's player color.
  Color get_color() const;

  struct Hash {
    uint64_t hash;
    constexpr bool operator==(const Hash &other) const { return hash == other.hash; }
    constexpr bool is_null() const { return hash == 0; }
    // A deterministic mapping from Hash to an integer in [0, n).
    constexpr size_t to_index(size_t n) const { return hash % n; }

    // A null hash (highly likely) differs from the hash of any valid board.
    static const Hash null;
  };
  // Returns a hash of this board.
  Hash get_hash() const;

  // Returns true if the game has ended.
  bool is_game_over() const;

  // Returns true if the game has ended, accounting for threefold repetition.
  template <typename RepetitionTracker>
    requires IsRepetitionTracker<RepetitionTracker>
  bool is_game_over(const RepetitionTracker &repetition_tracker) const;

  // Returns std::nullopt if the game has not ended.
  // Else returns the score (-1 if black won, 0 if draw, 1 if white won).
  std::optional<int32_t> get_score() const;

  // Returns std::nullopt if the game has not ended.
  // Else returns the score (-1 if black won, 0 if draw, 1 if white won).
  // This accounts for threefold repetition.
  template <typename RepetitionTracker>
    requires IsRepetitionTracker<RepetitionTracker>
  std::optional<int32_t> get_score(const RepetitionTracker &repetition_tracker) const;

private:
  Player white;
  Player black;
  // The square that is currently capturable by en passant (if any).
  Bitboard en_passant_bit;
  // Keeps track of number of plies without capture / pawn pushes (for fifty move rule).
  int16_t halfmove_clock;
  bool is_white_turn;
};

// ========== IMPLEMENTATIONS ==========

constexpr Board::Hash Board::Hash::null{0};

template <typename RepetitionTracker>
  requires IsRepetitionTracker<RepetitionTracker>
bool Board::is_game_over(const RepetitionTracker &repetition_tracker) const {
  return get_score(repetition_tracker).has_value();
}

template <typename RepetitionTracker>
  requires IsRepetitionTracker<RepetitionTracker>
std::optional<int32_t> Board::get_score(const RepetitionTracker &repetition_tracker) const {
  if (repetition_tracker.is_repetition_draw()) return 0;
  return get_score();
}

}  // namespace chess