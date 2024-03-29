#include "board.h"

#include <cctype>

#include "move_gen.h"

Board::Board(Player white, Player black, u64 en_passant_bit, bool is_white_turn)
    : white{white}, black{black}, en_passant_bit{en_passant_bit}, is_white_turn{is_white_turn} {}

Board Board::initial() { return Board(Player::white_initial(), Player::black_initial(), 0, true); }

Board Board::from_epd(std::string_view epd) {
  Player white{0, 0, 0, 0, 0, 0, false, false};
  Player black{0, 0, 0, 0, 0, 0, false, false};
  size_t index = 0;

  // Pieces.
  for (int y = 7; y >= 0; y--) {
    int x = 0;
    while (epd[index] != '/' && epd[index] != ' ') {
      char ch = epd[index];
      index++;
      if (std::isdigit(ch)) {
        x += static_cast<int>(ch - '0');
        continue;
      }
      bool is_white_piece = std::isupper(ch);
      PieceVariant piece = piece_variant::from_char(ch);
      Player& player = is_white_piece ? white : black;
      player[piece] |= u64(1) << (y * 8 + x);
      x++;
    }
    index++;
  }

  // Side to move.
  bool is_white_turn = epd[index] == 'w';
  index += 2;

  // Castling rights.
  while (epd[index] != '-' && epd[index] != ' ') {
    bool is_white = std::isupper(epd[index]);
    Player& player = is_white ? white : black;
    if (epd[index] == 'K' || epd[index] == 'k') player.enable_kingside_castling();
    else player.enable_queenside_castling();
    index++;
  }
  if (epd[index] == '-') index++;
  index++;

  // En passant target square.
  u64 en_passant_bit = 0;
  if (epd[index] != '-') en_passant_bit = bit::from_algebraic(epd.substr(index, 2));

  return Board{white, black, en_passant_bit, is_white_turn};
}

Board Board::apply_move(const Move& move) const {
  Board board = *this;
  Player& cur = board.cur_player();
  Player& opp = board.opp_player();
  const PieceVariant piece = move.get_piece();
  const u64 from = move.get_from();
  const u64 to = move.get_to();

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
  board.en_passant_bit = 0;
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

  board.is_white_turn = !board.is_white_turn;

  return board;
}

Board Board::apply_uci_move(std::string_view uci_move) {
  u64 from = bit::from_algebraic(uci_move.substr(0, 2));
  u64 to = bit::from_algebraic(uci_move.substr(2, 2));
  if (uci_move.size() == 5) {
    PieceVariant promotion_piece = piece_variant::from_char(uci_move[4]);
    return apply_move(Move{from, to, promotion_piece, opp_player().piece_at(to)});
  } else {
    PieceVariant piece = cur_player().piece_at(from);
    return apply_move(Move{piece, from, to, opp_player().piece_at(to)});
  }
}

const Player& Board::cur_player() const { return is_white_turn ? white : black; }

Player& Board::cur_player() { return is_white_turn ? white : black; }

MoveContainer Board::generate_quiescence_moves() const { return move_gen::generate_quiescence_moves(*this); }

MoveContainer Board::generate_quiescence_moves_and_checks() const {
  return move_gen::generate_quiescence_moves_and_checks(*this);
}

MoveContainer Board::generate_moves() const { return move_gen::generate_moves(*this); }

bool Board::has_moves() const { return move_gen::has_moves(*this); }

bool Board::is_a_check(const Move& move) const {
  // This function is not optimized as it should only be called in non-critical code for external interaction.
  return apply_move(move).is_in_check();
}

bool Board::is_in_check() const { return is_under_attack(cur_player()[PieceVariant::King]); }

bool Board::is_under_attack(u64 square) const { return move_gen::is_under_attack(*this, square); }

const Player& Board::get_white() const { return white; }

const Player& Board::get_black() const { return black; }

const Player& Board::opp_player() const { return is_white_turn ? black : white; }

Player& Board::opp_player() { return is_white_turn ? black : white; }

Board Board::skip_turn() const {
  Board new_board = *this;
  new_board.is_white_turn = !new_board.is_white_turn;
  new_board.en_passant_bit = 0;
  return new_board;
}

u64 Board::get_en_passant() const { return en_passant_bit; }

std::string Board::to_string() const {
  std::string s;
  for (int y = 7; y >= 0; y--) {
    for (int x = 0; x < 8; x++) {
      u64 bit = u64(1) << (y * 8 + x);
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