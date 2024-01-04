#include "board.h"

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
      if (ch >= '0' && ch <= '9') {
        x += static_cast<int>(ch - '0');
        continue;
      }
      bool is_white_piece = ch >= 'A' && ch <= 'Z';
      if (is_white_piece) ch += 'a' - 'A';
      Piece piece = Piece::Bishop;
      if (ch == 'b')
        piece = Piece::Bishop;
      else if (ch == 'k')
        piece = Piece::King;
      else if (ch == 'n')
        piece = Piece::Knight;
      else if (ch == 'p')
        piece = Piece::Pawn;
      else if (ch == 'q')
        piece = Piece::Queen;
      else if (ch == 'r')
        piece = Piece::Rook;
      Player& player = is_white_piece ? white : black;
      player.mut_bitboard(piece) |= u64(1) << (y * 8 + x);
      x++;
    }
    index++;
  }

  // Side to move.
  bool is_white_turn = epd[index] == 'w';
  index += 2;

  // Castling rights.
  while (epd[index] != '-' && epd[index] != ' ') {
    bool is_white = epd[index] >= 'A' && epd[index] <= 'Z';
    Player& player = is_white ? white : black;
    if (epd[index] == 'K' || epd[index] == 'k')
      player.enable_kingside_castling();
    else
      player.enable_queenside_castling();
    index++;
  }
  index++;

  // En passant target square.
  u64 en_passant_bit = 0;
  if (epd[index] != '-') {
    en_passant_bit = bit::from_algebraic(epd.substr(index, 2));
    // EPD stores the attacked square, while we need the pawn itself.
    if (en_passant_bit & bitboard::RANK_2)
      en_passant_bit <<= 8;
    else
      en_passant_bit >>= 8;
  }

  return Board{white, black, en_passant_bit, is_white_turn};
}

Board Board::apply_move(Piece piece, u64 from, u64 to) const {
  Board board = *this;
  Player& cur = board.cur_player();
  Player& opp = board.opp_player();
  cur.mut_bitboard(piece) ^= from | to;

  if (opp.get_bitboard(Piece::Rook) & to) {
    if (opp.can_castle_kingside() && to == is_white_turn ? bitboard::H1 : bitboard::H8) {
      opp.disable_kingside_castling();
    }
    if (opp.can_castle_queenside() && to == is_white_turn ? bitboard::A1 : bitboard::A8) {
      opp.disable_queenside_castling();
    }
  }
  opp &= ~to;

  // Handle castling.
  if (piece == Piece::King) {
    if (to == from << 2) {  // Kingside castling.
      cur.mut_bitboard(Piece::Rook) ^= from << 1 | from << 3;
    } else if (to == from >> 2) {  // Queenside castling.
      cur.mut_bitboard(Piece::Rook) ^= from >> 1 | from >> 4;
    }
  }

  // Handle en passant's capture.
  if (piece == Piece::Pawn && to == (is_white_turn ? en_passant_bit << 8 : en_passant_bit >> 8)) {
    opp.mut_bitboard(Piece::Pawn) ^= en_passant_bit;
  }

  // Update en passant flag.
  board.en_passant_bit = piece == Piece::Pawn && (to == from << 16 || to == from >> 16) ? to : 0;

  // Update castling flag.
  if (piece == Piece::King) cur.disable_castling();
  if (piece == Piece::Rook) {
    if (from == is_white_turn ? bitboard::H1 : bitboard::H8) cur.disable_kingside_castling();
    if (from == is_white_turn ? bitboard::A1 : bitboard::A8) cur.disable_queenside_castling();
  }

  board.is_white_turn = !board.is_white_turn;

  return board;
}

Board Board::apply_promotion(u64 from, u64 to, Piece piece) const {
  Board board = *this;
  Player& cur = board.cur_player();
  Player& opp = board.opp_player();
  cur.mut_bitboard(Piece::Pawn) ^= from;
  cur.mut_bitboard(piece) ^= to;

  if (opp.get_bitboard(Piece::Rook) & to) {
    if (opp.can_castle_kingside() && to == is_white_turn ? bitboard::H1 : bitboard::H8) {
      opp.disable_kingside_castling();
    }
    if (opp.can_castle_queenside() && to == is_white_turn ? bitboard::A1 : bitboard::A8) {
      opp.disable_queenside_castling();
    }
  }
  opp &= ~to;

  // Update en passant flag.
  board.en_passant_bit = 0;

  board.is_white_turn = !board.is_white_turn;

  return board;
}

Board Board::apply_uci_move(std::string_view uci_move) {
  u64 from = bit::from_algebraic(uci_move.substr(0, 2));
  u64 to = bit::from_algebraic(uci_move.substr(2, 2));
  if (uci_move.size() == 5) {
    Piece promotion_piece = Piece::Bishop;
    switch (uci_move[4]) {
      case 'b':
        promotion_piece = Piece::Bishop;
        break;
      case 'n':
        promotion_piece = Piece::Knight;
        break;
      case 'q':
        promotion_piece = Piece::Queen;
        break;
      case 'r':
        promotion_piece = Piece::Rook;
        break;
      default:
        throw "Unreachable - Board::apply_uci_move";
    }
    return apply_promotion(from, to, promotion_piece);
  } else {
    Piece piece = cur_player().piece_at(from);
    return apply_move(piece, from, to);
  }
}

const Player& Board::cur_player() const { return is_white_turn ? white : black; }

Player& Board::cur_player() { return is_white_turn ? white : black; }

MoveSet Board::generate_moves() const {
  MoveSet move_set{*this};
  generate_bishop_moves(move_set);
  generate_king_moves(move_set);
  generate_knight_moves(move_set);
  generate_pawn_moves(move_set);
  generate_queen_moves(move_set);
  generate_rook_moves(move_set);
  return move_set;
}

bool Board::is_in_check() const { return is_under_attack(cur_player().get_bitboard(Piece::King)); }

bool Board::is_under_attack(u64 square) const {
  const Player& opp = opp_player();
  u64 cur_occupied = cur_player().occupied();
  u64 opp_occupied = opp_player().occupied();
  u64 total_occupied = cur_occupied | opp_occupied;
  if (bitboard::bishop_attacks(square, total_occupied) &
      (opp.get_bitboard(Piece::Bishop) | opp.get_bitboard(Piece::Queen))) {
    return true;
  }
  if (bitboard::king_attacks(square) & opp.get_bitboard(Piece::King)) return true;
  if (bitboard::knight_attacks(square) & opp.get_bitboard(Piece::Knight)) return true;
  if (bitboard::pawn_attacks(square, is_white_turn) & opp.get_bitboard(Piece::Pawn)) return true;
  if (bitboard::rook_attacks(square, total_occupied) &
      (opp.get_bitboard(Piece::Rook) | opp.get_bitboard(Piece::Queen))) {
    return true;
  }
  return false;
}

bool Board::is_white_to_move() const { return is_white_turn; }

bool Board::moved_into_check() const {
  const Player& cur = cur_player();
  const Player& opp = opp_player();
  u64 king = opp.get_bitboard(Piece::King);
  u64 cur_occupied = cur_player().occupied();
  u64 opp_occupied = opp_player().occupied();
  u64 total_occupied = cur_occupied | opp_occupied;
  if (bitboard::bishop_attacks(king, total_occupied) &
      (cur.get_bitboard(Piece::Bishop) | cur.get_bitboard(Piece::Queen))) {
    return true;
  }
  if (bitboard::king_attacks(king) & cur.get_bitboard(Piece::King)) return true;
  if (bitboard::knight_attacks(king) & cur.get_bitboard(Piece::Knight)) return true;
  if (bitboard::pawn_attacks(king, !is_white_turn) & cur.get_bitboard(Piece::Pawn)) return true;
  if (bitboard::rook_attacks(king, total_occupied) & (cur.get_bitboard(Piece::Rook) | cur.get_bitboard(Piece::Queen))) {
    return true;
  }
  return false;
}

const Player& Board::opp_player() const { return is_white_turn ? black : white; }

Player& Board::opp_player() { return is_white_turn ? black : white; }

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

void Board::generate_bishop_moves(MoveSet& move_set) const {
  u64 bishops = cur_player().get_bitboard(Piece::Bishop);
  u64 cur_occupied = cur_player().occupied();
  u64 opp_occupied = opp_player().occupied();
  u64 total_occupied = cur_occupied | opp_occupied;
  while (bishops) {
    u64 bishop = bit::lsb(bishops);
    bishops ^= bishop;
    u64 to_bitmap = bitboard::bishop_attacks(bishop, total_occupied) & ~cur_occupied;
    move_set.add_piece_moves(Piece::Bishop, bishop, to_bitmap);
  }
}

void Board::generate_king_moves(MoveSet& move_set) const {
  u64 king = cur_player().get_bitboard(Piece::King);
  u64 cur_occupied = cur_player().occupied();
  u64 opp_occupied = opp_player().occupied();
  u64 total_occupied = cur_occupied | opp_occupied;
  u64 to_bitmap = bitboard::king_attacks(king) & ~cur_occupied;
  if (cur_player().can_castle_kingside() && !(total_occupied & (king << 1 | king << 2)) && !is_under_attack(king) &&
      !is_under_attack(king << 1)) {
    to_bitmap |= king << 2;
  }
  if (cur_player().can_castle_queenside() && !(total_occupied & (king >> 1 | king >> 2 | king >> 3)) &&
      !is_under_attack(king) && !is_under_attack(king >> 1)) {
    to_bitmap |= king >> 2;
  }
  move_set.add_piece_moves(Piece::King, king, to_bitmap);
}

void Board::generate_knight_moves(MoveSet& move_set) const {
  u64 knights = cur_player().get_bitboard(Piece::Knight);
  u64 cur_occupied = cur_player().occupied();
  while (knights) {
    u64 knight = bit::lsb(knights);
    knights ^= knight;
    u64 to_bitmap = bitboard::knight_attacks(knight) & ~cur_occupied;
    move_set.add_piece_moves(Piece::Knight, knight, to_bitmap);
  }
}

void Board::generate_pawn_moves(MoveSet& move_set) const {
  u64 pawns = cur_player().get_bitboard(Piece::Pawn);
  u64 cur_occupied = cur_player().occupied();
  u64 opp_occupied = opp_player().occupied() | (is_white_turn ? en_passant_bit << 8 : en_passant_bit >> 8);
  u64 total_occupied = cur_occupied | opp_occupied;
  while (pawns) {
    u64 pawn = bit::lsb(pawns);
    pawns ^= pawn;
    u64 to_bitmap = bitboard::pawn_attacks(pawn, is_white_turn) & opp_occupied;  // Captures.
    u64 push = is_white_turn ? pawn << 8 : pawn >> 8;
    u64 double_push = is_white_turn ? (pawn & bitboard::RANK_2) << 16 : (pawn & bitboard::RANK_7) >> 16;
    to_bitmap |= push & ~total_occupied;
    to_bitmap |= (push & ~total_occupied) ? (double_push & ~total_occupied) : 0;
    move_set.add_piece_moves(Piece::Pawn, pawn, to_bitmap);
  }
}

void Board::generate_queen_moves(MoveSet& move_set) const {
  u64 queens = cur_player().get_bitboard(Piece::Queen);
  u64 cur_occupied = cur_player().occupied();
  u64 opp_occupied = opp_player().occupied();
  u64 total_occupied = cur_occupied | opp_occupied;
  while (queens) {
    u64 queen = bit::lsb(queens);
    queens ^= queen;
    u64 to_bitmap = (bitboard::bishop_attacks(queen, total_occupied) | bitboard::rook_attacks(queen, total_occupied)) &
                    ~cur_occupied;
    move_set.add_piece_moves(Piece::Queen, queen, to_bitmap);
  }
}

void Board::generate_rook_moves(MoveSet& move_set) const {
  u64 rooks = cur_player().get_bitboard(Piece::Rook);
  u64 cur_occupied = cur_player().occupied();
  u64 opp_occupied = opp_player().occupied();
  u64 total_occupied = cur_occupied | opp_occupied;
  while (rooks) {
    u64 rook = bit::lsb(rooks);
    rooks ^= rook;
    u64 to_bitmap = bitboard::rook_attacks(rook, total_occupied) & ~cur_occupied;
    move_set.add_piece_moves(Piece::Rook, rook, to_bitmap);
  }
}
