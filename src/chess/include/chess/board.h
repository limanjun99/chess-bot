#pragma once

#include <concepts>
#include <optional>
#include <string_view>
#include <vector>

#include "bitboard.h"
#include "move_container.h"
#include "player.h"

namespace chess {

// clang-format off
template <typename T>
concept IsRepetitionTracker = requires(T repetition_tracker) {
  { repetition_tracker.is_repetition_draw() } -> std::convertible_to<bool>;
};
// clang-format on

class Board {
public:
  constexpr explicit Board(Player white, Player black, Bitboard en_passant_bit, bool is_white_turn,
                           int16_t halfmove_clock = 0);

  // Starting board of a chess game.
  static constexpr Board initial();

  // Construct a board from FEN.
  // https://www.chessprogramming.org/Forsyth-Edwards_Notation
  static Board from_fen(std::string_view fen);

  // Returns a new board that is the result of applying the given move.
  constexpr Board apply_move(const Move &move) const;

  // Returns a reference to the current player whose turn it is.
  constexpr const Player &cur_player() const;
  constexpr Player &cur_player();

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
  constexpr const Player &opp_player() const;
  constexpr Player &opp_player();

  // Return a new board, where the current player skipped their turn.
  constexpr Board skip_turn() const;

  template <Color PlayerColor>
  constexpr const Player &get_player() const;

  // Returns the bitboard for the en passant bit.
  constexpr Bitboard get_en_passant() const;

  // Returns a 8x8 newline delimited string represenation of the board.
  constexpr std::string to_string() const;

  // Returns the current turn's player color.
  constexpr Color get_color() const;

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
  constexpr Hash get_hash() const;

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

  // Two boards are equal if they have the same position and flags.
  constexpr bool operator==(const Board &other) const;

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

constexpr Board::Board(Player white, Player black, Bitboard en_passant_bit, bool is_white_turn, int16_t halfmove_clock)
    : white{white},
      black{black},
      en_passant_bit{en_passant_bit},
      halfmove_clock{halfmove_clock},
      is_white_turn{is_white_turn} {}

constexpr Board Board::initial() {
  return Board(Player::white_initial(), Player::black_initial(), Bitboard::empty, true);
}

constexpr Board Board::apply_move(const Move &move) const {
  Board board = *this;
  Player &cur = board.cur_player();
  Player &opp = board.opp_player();
  const PieceType piece = move.get_piece();
  const Bitboard from = move.get_from();
  const Bitboard to = move.get_to();

  cur[piece] ^= from | to;

  // Update castling flag.
  if (move.get_captured_piece() == PieceType::Rook) {
    if (to == (is_white_turn ? Bitboard::H8 : Bitboard::H1)) opp.disable_kingside_castling();
    if (to == (is_white_turn ? Bitboard::A8 : Bitboard::A1)) opp.disable_queenside_castling();
  }
  if (piece == PieceType::King) {
    cur.disable_castling();
  } else if (piece == PieceType::Rook) {
    if (from == (is_white_turn ? Bitboard::H1 : Bitboard::H8)) cur.disable_kingside_castling();
    if (from == (is_white_turn ? Bitboard::A1 : Bitboard::A8)) cur.disable_queenside_castling();
  }

  if (move.is_capture()) {
    opp[move.get_captured_piece()] &= ~to;
  }

  // Handle castling.
  if (piece == PieceType::King) {
    if (to == from << 2) {  // Kingside castling.
      cur[PieceType::Rook] ^= from << 1 | from << 3;
    } else if (to == from >> 2) {  // Queenside castling.
      cur[PieceType::Rook] ^= from >> 1 | from >> 4;
    }
  }

  // Handle en passant's capture.
  if (piece == PieceType::Pawn && to == en_passant_bit) {
    opp[PieceType::Pawn] ^= is_white_turn ? en_passant_bit >> 8 : en_passant_bit << 8;
  }

  // Update en passant flag.
  board.en_passant_bit = Bitboard::empty;
  if (piece == PieceType::Pawn) {
    if (to == from << 16) {
      board.en_passant_bit = from << 8;
    } else if (to == from >> 16) {
      board.en_passant_bit = from >> 8;
    }
  }

  // Handle promotions.
  if (move.is_promotion()) {
    cur[move.get_promotion_piece()] ^= to;
    cur[PieceType::Pawn] ^= to;
  }

  // Update halfmove clock.
  if (move.get_piece() == PieceType::Pawn || move.is_capture()) board.halfmove_clock = 0;
  else board.halfmove_clock++;

  board.is_white_turn = !board.is_white_turn;

  return board;
}

constexpr const Player &Board::cur_player() const { return is_white_turn ? white : black; }

constexpr Player &Board::cur_player() { return is_white_turn ? white : black; }

constexpr const Player &Board::opp_player() const { return is_white_turn ? black : white; }

constexpr Player &Board::opp_player() { return is_white_turn ? black : white; }

constexpr Board Board::skip_turn() const {
  Board new_board = *this;
  new_board.is_white_turn = !new_board.is_white_turn;
  new_board.en_passant_bit = Bitboard::empty;
  return new_board;
}

template <Color PlayerColor>
constexpr const Player &Board::get_player() const {
  if constexpr (PlayerColor == Color::White) return white;
  else return black;
}

constexpr Bitboard Board::get_en_passant() const { return en_passant_bit; }

constexpr std::string Board::to_string() const {
  std::string s;
  for (int y{7}; y >= 0; y--) {
    for (int x{0}; x < 8; x++) {
      Bitboard bit{Bitboard::from_coordinate(y, x)};
      if (white.occupied() & bit) {
        s += piece::to_colored_char<Color::White>(white.piece_at(bit));
      } else if (black.occupied() & bit) {
        s += piece::to_colored_char<Color::Black>(black.piece_at(bit));
      } else {
        s += '.';
      }
    }
    if (y) s += '\n';
  }
  return s;
}

constexpr Color Board::get_color() const { return is_white_turn ? Color::White : Color::Black; }

constexpr Board::Hash Board::Hash::null{0};

constexpr Board::Hash Board::get_hash() const {
  // This uses a polynomial rolling hash (modulo 1<<64).
  // I have no idea if this sufficiently collision resistant, but it is faster than Zobrist hashing.
  const uint64_t prime{888888877777777};
  uint64_t hash{0};
  hash += is_white_turn;
  hash *= prime;
  hash += static_cast<uint64_t>(en_passant_bit);
  hash *= prime;
  for (const Player &player : {get_player<Color::White>(), get_player<Color::Black>()}) {
    hash += player.can_castle_kingside();
    hash *= prime;
    hash += player.can_castle_queenside();
    hash *= prime;
    for (int piece{0}; piece < 6; piece++) {
      hash += static_cast<uint64_t>(player[static_cast<PieceType>(piece)]);
      hash *= prime;
    }
  }
  return Hash{hash};
}

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

constexpr bool Board::operator==(const Board &other) const {
  return white == other.white && black == other.black && en_passant_bit == other.en_passant_bit &&
         halfmove_clock == other.halfmove_clock && is_white_turn == other.is_white_turn;
}

}  // namespace chess