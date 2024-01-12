#pragma once

#include <cstdint>

enum class Piece : int32_t {
  Bishop = 0,
  King = 1,
  Knight = 2,
  Pawn = 3,
  Queen = 4,
  Rook = 5,
  None = 6,  // Piece::None is preferred over std::optional<Piece> as it takes up 4 bytes (as compared to 8 bytes).
};

namespace piece {
// Returns true if the piece is a bishop, queen or rook.
bool is_slider(Piece piece);

// Convert a piece to the corresponding lowercase char. The piece must not be None.
char to_char(Piece piece);

// Convert a piece to the corresponding char (uppercase for white, lowercase for black). The piece must not be None.
char to_colored_char(Piece piece, bool is_white);

// Convert a char (either uppercase / lowercase) to the corresponding piece.
Piece from_char(char c);
}