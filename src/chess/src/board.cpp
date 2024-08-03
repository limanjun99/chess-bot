#include "board.h"

#include <array>
#include <cctype>
#include <random>
#include <sstream>
#include <type_traits>

#include "bitboard.h"
#include "constants.h"
#include "move_gen.h"
#include "piece.h"

using namespace chess;

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
      PieceType piece = piece::from_char(ch);
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
  Bitboard en_passant_bit{Bitboard::empty};
  if (buffer[0] != '-') en_passant_bit = Bitboard::from_algebraic(buffer);

  // Halfmove clock.
  int16_t halfmove_clock;
  fen_stream >> halfmove_clock;

  // Fullmove counter (ignored as it's not necessary for engines).

  return Board{white, black, en_passant_bit, is_white_turn, halfmove_clock};
}

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

bool Board::is_in_check() const { return is_under_attack(cur_player()[PieceType::King]); }

bool Board::is_under_attack(Bitboard square) const { return move_gen::is_under_attack(*this, square); }

bool Board::is_game_over() const { return get_score().has_value(); }

std::optional<int32_t> Board::get_score() const {
  if (halfmove_clock >= constants::fifty_move_rule_plies) return 0;  // Draw by fifty move rule.
  if (has_moves()) return std::nullopt;                              // Game not over.
  if (!is_in_check()) return 0;                                      // Stalemate.
  if (is_white_turn) return -1;                                      // White is checkmated.
  else return 1;                                                     // Black is checkmated.
}

std::string Board::to_fen() const {
  auto fen = std::string{};

  // Pieces.
  for (auto y = 7; y >= 0; y--) {
    auto empty_count = 0;
    for (auto x = 0; x < 8; x++) {
      const auto bit = Bitboard::from_coordinate(y, x);
      const auto white_piece = get_player<Color::White>().piece_at(bit);
      const auto black_piece = get_player<Color::Black>().piece_at(bit);

      if (white_piece == PieceType::None && black_piece == PieceType::None) {
        empty_count++;
        continue;
      }

      if (empty_count > 0) {
        fen += std::to_string(empty_count);
        empty_count = 0;
      }

      const auto piece_char = [&]() {
        if (white_piece != PieceType::None) return piece::to_colored_char<Color::White>(white_piece);
        return piece::to_colored_char<Color::Black>(black_piece);
      }();
      fen += piece_char;
    }

    if (empty_count) fen += std::to_string(empty_count);
    if (y) fen += "/";
  }

  // Side to move.
  fen += " ";
  fen += is_white_turn ? "w" : "b";

  // Castling rights.
  fen += " ";
  if (get_player<Color::White>().can_castle_kingside()) fen += "K";
  if (get_player<Color::White>().can_castle_queenside()) fen += "Q";
  if (get_player<Color::Black>().can_castle_kingside()) fen += "k";
  if (get_player<Color::Black>().can_castle_queenside()) fen += "q";
  if (fen.ends_with(' ')) fen += "-";

  // En passant target square.
  fen += " ";
  if (en_passant_bit) fen += en_passant_bit.to_algebraic();
  else fen += "-";

  // Halfmove clock.
  fen += " ";
  fen += std::to_string(halfmove_clock);

  return fen;
}
