#include "piece.h"

#include <cctype>

constexpr char piece_chars[6] = {'b', 'k', 'n', 'p', 'q', 'r'};

bool piece::is_slider(Piece piece) { return piece == Piece::Bishop || piece == Piece::Queen || piece == Piece::Rook; }

char piece::to_char(Piece piece) { return piece_chars[static_cast<int>(piece)]; }

char piece::to_colored_char(Piece piece, bool is_white) {
  if (is_white) return std::toupper(to_char(piece));
  return to_char(piece);
}

Piece piece::from_char(char c) {
  switch (std::tolower(c)) {
    case 'b':
      return Piece::Bishop;
    case 'k':
      return Piece::King;
    case 'n':
      return Piece::Knight;
    case 'p':
      return Piece::Pawn;
    case 'q':
      return Piece::Queen;
    case 'r':
      return Piece::Rook;
    default:
      throw "Unreachable";
  }
}