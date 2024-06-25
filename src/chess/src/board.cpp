#include "board.h"

#include <array>
#include <cctype>
#include <random>
#include <sstream>
#include <type_traits>

#include "bitboard.h"
#include "constants.h"
#include "move_gen.h"

using namespace chess;

Board::Board(Player white, Player black, Bitboard en_passant_bit, bool is_white_turn, int16_t halfmove_clock)
    : white{white},
      black{black},
      en_passant_bit{en_passant_bit},
      halfmove_clock{halfmove_clock},
      is_white_turn{is_white_turn} {}

Board Board::initial() { return Board(Player::white_initial(), Player::black_initial(), bitboard::EMPTY, true); }

Board Board::from_fen(std::string_view fen) {
  Player white{Player::empty()};
  Player black{Player::empty()};

  // Use stringstream (instead of manually parsing) for clarity,
  // as this function need not be performant.
  std::istringstream fen_stream{std::string{fen}};
  std::string buffer;

  // Pieces.
  fen_stream >> buffer;
  std::istringstream pieces_stream{std::move(buffer)};
  for (int y = 7; y >= 0; y--) {
    std::string line;
    std::getline(pieces_stream, line, '/');
    int x = 0;
    for (char ch : line) {
      if (std::isdigit(ch)) {
        x += static_cast<int>(ch - '0');
        continue;
      }
      bool is_white_piece = std::isupper(ch);
      PieceVariant piece = piece_variant::from_char(ch);
      Player &player = is_white_piece ? white : black;
      player[piece] |= Bitboard::from_coordinate(y, x);
      x++;
    }
  }

  // Side to move.
  fen_stream >> buffer;
  bool is_white_turn = buffer[0] == 'w';

  // Castling rights.
  fen_stream >> buffer;
  for (char ch : buffer) {
    if (ch == '-') continue;
    bool is_white_piece = std::isupper(ch);
    Player &player = is_white_piece ? white : black;
    if (ch == 'K' || ch == 'k') player.enable_kingside_castling();
    else player.enable_queenside_castling();
  }

  // En passant target square.
  fen_stream >> buffer;
  Bitboard en_passant_bit{bitboard::EMPTY};
  if (buffer[0] != '-') en_passant_bit = Bitboard::from_algebraic(buffer);

  // Halfmove clock.
  int16_t halfmove_clock;
  fen_stream >> halfmove_clock;

  // Fullmove counter (ignored as it's not necessary for engines).

  return Board{white, black, en_passant_bit, is_white_turn, halfmove_clock};
}

Board Board::apply_move(const Move &move) const {
  Board board = *this;
  Player &cur = board.cur_player();
  Player &opp = board.opp_player();
  const PieceVariant piece = move.get_piece();
  const Bitboard from = move.get_from();
  const Bitboard to = move.get_to();

  cur[piece] ^= from | to;

  // Update castling flag.
  if (move.get_captured_piece() == PieceVariant::Rook) {
    if (to == (is_white_turn ? bitboard::H8 : bitboard::H1)) opp.disable_kingside_castling();
    if (to == (is_white_turn ? bitboard::A8 : bitboard::A1)) opp.disable_queenside_castling();
  }
  if (piece == PieceVariant::King) {
    cur.disable_castling();
  } else if (piece == PieceVariant::Rook) {
    if (from == (is_white_turn ? bitboard::H1 : bitboard::H8)) cur.disable_kingside_castling();
    if (from == (is_white_turn ? bitboard::A1 : bitboard::A8)) cur.disable_queenside_castling();
  }

  if (move.is_capture()) {
    opp[move.get_captured_piece()] &= ~to;
  }

  // Handle castling.
  if (piece == PieceVariant::King) {
    if (to == from << 2) {  // Kingside castling.
      cur[PieceVariant::Rook] ^= from << 1 | from << 3;
    } else if (to == from >> 2) {  // Queenside castling.
      cur[PieceVariant::Rook] ^= from >> 1 | from >> 4;
    }
  }

  // Handle en passant's capture.
  if (piece == PieceVariant::Pawn && to == en_passant_bit) {
    opp[PieceVariant::Pawn] ^= is_white_turn ? en_passant_bit >> 8 : en_passant_bit << 8;
  }

  // Update en passant flag.
  board.en_passant_bit = bitboard::EMPTY;
  if (piece == PieceVariant::Pawn) {
    if (to == from << 16) {
      board.en_passant_bit = from << 8;
    } else if (to == from >> 16) {
      board.en_passant_bit = from >> 8;
    }
  }

  // Handle promotions.
  if (move.is_promotion()) {
    cur[move.get_promotion_piece()] ^= to;
    cur[PieceVariant::Pawn] ^= to;
  }

  // Update halfmove clock.
  board.halfmove_clock++;
  if (move.get_piece() == PieceVariant::Pawn || move.is_capture()) board.halfmove_clock = 0;

  board.is_white_turn = !board.is_white_turn;

  return board;
}

const Player &Board::cur_player() const { return is_white_turn ? white : black; }

Player &Board::cur_player() { return is_white_turn ? white : black; }

MoveContainer Board::generate_quiescence_moves() const { return move_gen::generate_quiescence_moves(*this); }

MoveContainer Board::generate_quiescence_moves_and_checks() const {
  return move_gen::generate_quiescence_moves_and_checks(*this);
}

MoveContainer Board::generate_moves() const { return move_gen::generate_moves(*this); }

bool Board::has_moves() const { return move_gen::has_moves(*this); }

bool Board::is_a_check(const Move &move) const {
  // This function is not optimized as it should only be called in non-critical code for external interaction.
  return apply_move(move).is_in_check();
}

bool Board::is_in_check() const { return is_under_attack(cur_player()[PieceVariant::King]); }

bool Board::is_under_attack(Bitboard square) const { return move_gen::is_under_attack(*this, square); }

const Player &Board::get_white() const { return white; }

const Player &Board::get_black() const { return black; }

const Player &Board::opp_player() const { return is_white_turn ? black : white; }

Player &Board::opp_player() { return is_white_turn ? black : white; }

Board Board::skip_turn() const {
  Board new_board = *this;
  new_board.is_white_turn = !new_board.is_white_turn;
  new_board.en_passant_bit = bitboard::EMPTY;
  return new_board;
}

Bitboard Board::get_en_passant() const { return en_passant_bit; }

std::string Board::to_string() const {
  std::string s;
  for (int y = 7; y >= 0; y--) {
    for (int x = 0; x < 8; x++) {
      Bitboard bit = Bitboard::from_coordinate(y, x);
      constexpr char piece_char[6] = {'b', 'k', 'n', 'p', 'q', 'r'};
      if (white.occupied() & bit) {
        s += piece_char[static_cast<int>(white.piece_at(bit))] - 32;
      } else if (black.occupied() & bit) {
        s += piece_char[static_cast<int>(black.piece_at(bit))];
      } else {
        s += '.';
      }
    }
    if (y) s += '\n';
  }
  return s;
}

Color Board::get_color() const { return is_white_turn ? Color::White : Color::Black; }

Board::Hash Board::get_hash() const {
  // This uses a polynomial rolling hash (modulo 1<<64).
  // I have no idea if this sufficiently collision resistant, but it is faster than Zobrist hashing.
  const uint64_t prime{888888877777777};
  uint64_t hash{0};
  hash += is_white_turn;
  hash *= prime;
  hash += static_cast<uint64_t>(en_passant_bit);
  hash *= prime;
  for (const Player &player : {get_white(), get_black()}) {
    hash += player.can_castle_kingside();
    hash *= prime;
    hash += player.can_castle_queenside();
    hash *= prime;
    for (int piece{0}; piece < 6; piece++) {
      hash += static_cast<uint64_t>(player[static_cast<PieceVariant>(piece)]);
      hash *= prime;
    }
  }
  return Hash{hash};
}

bool Board::is_game_over() const { return get_score().has_value(); }

std::optional<int32_t> Board::get_score() const {
  if (halfmove_clock >= fifty_move_rule_plies) return 0;  // Draw by fifty move rule.
  if (has_moves()) return std::nullopt;                   // Game not over.
  if (!is_in_check()) return 0;                           // Stalemate.
  if (is_white_turn) return -1;                           // White is checkmated.
  else return 1;                                          // Black is checkmated.
}
