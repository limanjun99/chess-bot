#include "board.h"

#include <array>
#include <cctype>
#include <random>
#include <sstream>

#include "bitboard.h"
#include "move_gen.h"

using namespace chess;

Board::Board(Player white, Player black, Bitboard en_passant_bit, bool is_white_turn, int16_t halfmove_clock)
    : white{white},
      black{black},
      en_passant_bit{en_passant_bit},
      tracked_positions{},
      halfmove_clock{halfmove_clock},
      is_white_turn{is_white_turn} {
  tracked_positions.push_back(get_hash());
}

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
      Player& player = is_white_piece ? white : black;
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
    Player& player = is_white_piece ? white : black;
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

Board Board::apply_move(const Move& move) const {
  Board board = *this;
  Player& cur = board.cur_player();
  Player& opp = board.opp_player();
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

  // Update tracked positions.
  if (move.get_piece() == PieceVariant::Pawn || move.is_capture()) board.tracked_positions.clear();
  // Only keep at most 12 positions to save memory, as threefold is unlikely with positions that occured too early.
  if (board.tracked_positions.size() >= 12) board.tracked_positions.erase(board.tracked_positions.begin());
  board.tracked_positions.push_back(board.get_hash());

  return board;
}

Board Board::apply_uci_move(std::string_view uci_move) {
  Bitboard from{Bitboard::from_algebraic(uci_move.substr(0, 2))};
  Bitboard to{Bitboard::from_algebraic(uci_move.substr(2, 2))};
  if (uci_move.size() == 5) {
    PieceVariant promotion_piece = piece_variant::from_char(uci_move[4]);
    return apply_move(Move::promotion(from, to, promotion_piece, opp_player().piece_at(to)));
  } else {
    PieceVariant piece = cur_player().piece_at(from);
    return apply_move(Move::move(from, to, piece, opp_player().piece_at(to)));
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

bool Board::is_under_attack(Bitboard square) const { return move_gen::is_under_attack(*this, square); }

const Player& Board::get_white() const { return white; }

const Player& Board::get_black() const { return black; }

const Player& Board::opp_player() const { return is_white_turn ? black : white; }

Player& Board::opp_player() { return is_white_turn ? black : white; }

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

uint64_t Board::get_hash() const {
  // Implementation follows https://www.chessprogramming.org/Zobrist_Hashing
  static std::mt19937_64 gen64{0};
  static const std::array<uint64_t, 2> color_rng{gen64(), gen64()};
  static const std::array<std::array<std::array<uint64_t, 64>, 6>, 2> piece_square_rng = []() {
    std::array<std::array<std::array<uint64_t, 64>, 6>, 2> piece_square_rng;
    for (auto& by_color : piece_square_rng) {
      for (auto& by_piece : by_color) {
        for (auto& by_square : by_piece) {
          by_square = gen64();
        }
      }
    }
    return piece_square_rng;
  }();
  static const std::array<uint64_t, 2> castle_kingside_rng{gen64(), gen64()};
  static const std::array<uint64_t, 2> castle_queenside_rng{gen64(), gen64()};
  static const std::array<uint64_t, 8> en_passant_file_rng{gen64(), gen64(), gen64(), gen64(),
                                                           gen64(), gen64(), gen64(), gen64()};

  uint64_t hash = color_rng[is_white_turn];
  for (int is_white = 0; is_white < 2; is_white++) {
    const Player& player = is_white ? get_white() : get_black();
    if (player.can_castle_kingside()) hash ^= castle_kingside_rng[is_white];
    if (player.can_castle_queenside()) hash ^= castle_queenside_rng[is_white];
    for (int piece = 0; piece < 6; piece++) {
      Bitboard bitboard = player[static_cast<PieceVariant>(piece)];
      for (const Bitboard bit : bitboard.iterate()) {
        hash ^= piece_square_rng[is_white][piece][bit.to_index()];
      }
    }
  }
  if (en_passant_bit) hash ^= en_passant_file_rng[en_passant_bit.to_index() % 8];
  return hash;
}

bool Board::is_game_over() const { return get_score().has_value(); }

std::optional<int32_t> Board::get_score() const {
  if (is_stagnant_draw()) return 0;
  if (has_moves()) return std::nullopt;  // Game not over.
  if (!is_in_check()) return 0;          // Stalemate.
  if (is_white_turn) return -1;          // White is checkmated.
  else return 1;                         // Black is checkmated.
}

bool Board::is_stagnant_draw() const {
  // Check fifty move rule.
  if (halfmove_clock >= 100) return true;

  // Check threefold.
  if (!tracked_positions.empty()) {
    const auto& current_position{tracked_positions.back()};
    int32_t count{0};
    for (const auto& position : tracked_positions) {
      if (position != current_position) continue;
      count++;
      if (count >= 3) return true;
    }
  }

  return false;
}